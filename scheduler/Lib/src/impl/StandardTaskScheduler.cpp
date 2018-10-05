#include <Scheduler/Lib/StandardTaskScheduler.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/TaskRunner.h>

#include <iostream>
#include <assert.h>

// #define SCHEDULER_DEBUGGING 1

Scheduler::Lib::StandardTaskScheduler::StandardTaskScheduler(
    const SchedulerParams& params,
    std::shared_ptr<ScheduleReporter>&& reporter,
    std::shared_ptr<TaskManager>&& manager,
    std::shared_ptr<Executor>&& executor)
    : m_executor(std::move(executor)),
      m_reporter(std::move(reporter)),
      m_manager(std::move(manager))
{ }

Scheduler::Lib::StandardTaskScheduler::~StandardTaskScheduler() { Shutdown(false); }

void Scheduler::Lib::StandardTaskScheduler::Enqueue(Chain* chain)
{
    ChainPtr chainPtr = chain->shared_from_this();

    if (!chain->IsValid())
    {
        Console(std::cout) << "Invalid chain '" << chain->ToString(true)
            << "' enqued to scheduler\n";
        return;
    }
    if (!chain->HasChildren())
    {
        Console(std::cout) << "Chain '" << chain->Id()
            << "' posted with no children\n";
    }

    std::unique_lock<std::mutex> lock(m_mutex);

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Enqueue chain: " << chain->Id() << '\n';
#endif  // SCHEDULER_DEBUGGING

    for (TaskPtr& child : chain->GetChildren()) EnqueueLocked(child, lock);
    m_queue.emplace_back(chain->Id());

    m_manager->Add(std::move(chainPtr));
    if (m_waiting) NotifyLocked(lock);
}

void Scheduler::Lib::StandardTaskScheduler::Enqueue(Task* task)
{
    TaskPtr taskPtr = task->shared_from_this();

    if (!task->IsValid())
    {
        Console(std::cout) << "Invalid task '" << task->ToString(true)
            << "' enqued to scheduler\n";
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    EnqueueLocked(std::move(taskPtr), lock);
}

void Scheduler::Lib::StandardTaskScheduler::EnqueueLocked(
    TaskPtr& task,
    std::unique_lock<std::mutex>& lock)
{
    assert(lock.owns_lock());
    TaskPtr taskPtr = task->shared_from_this();
    EnqueueLocked(std::move(taskPtr), lock);
}

void Scheduler::Lib::StandardTaskScheduler::EnqueueLocked(
    TaskPtr&& task,
    std::unique_lock<std::mutex>& lock)
{
    assert(lock.owns_lock());
    assert(task->IsValid());

    m_queue.emplace_back(task->Id());

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Enqueue: " << task->Id() << '\n';
#endif  // SCHEDULER_DEBUGGING

    m_manager->Add(std::move(task));
    if (m_waiting) NotifyLocked(lock);
}

Scheduler::Error Scheduler::Lib::StandardTaskScheduler::Initialize()
{
    return E_SUCCESS;
}

bool Scheduler::Lib::StandardTaskScheduler::IsTimedOut(
    const TaskPtr& task) const
{
    static Clock::duration TASK_TIMEOUT_INTERVAL = std::chrono::seconds(30);

    assert(task->IsValid());
    auto iter = m_timeouts.find(task->Id());
    if (iter == m_timeouts.end()) return false;
    return iter->second + TASK_TIMEOUT_INTERVAL > Clock::now();
}

bool Scheduler::Lib::StandardTaskScheduler::HandleTask(TaskPtr& task)
{
    assert(task != nullptr);
    assert(m_active.count(task->Id()) == 0);
    m_active.insert(task->Id());

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Enqueuing task '" << task->ToString(true)
        << "' with executor" << '\n';
#endif  // SCHEDULER_DEBUGGING

    std::weak_ptr<StandardTaskScheduler> self(shared_from_this());
    TaskRunnerPtr runner = std::make_shared<TaskRunner>(
        std::move(task),
        std::move(self));

    m_executor->Enqueue(runner);
    return true;
}

bool Scheduler::Lib::StandardTaskScheduler::HandleExpiredTask(TaskPtr& task)
{
    assert(task != nullptr);
    assert(task->IsExpired());

    if (task->IsActive())
    {
        Console(std::cout) << "Task '" << task->Id()
            << "' expired while running\n";
        // Cancel the task in the executor
        task->SetState(TaskState::CANCELLED);
    }
    else
    {
        Console(std::cout) << "Task '" << task->Id()
            << "' expired while in queue\n";
    }

    PrunePrematureTasks();
    m_manager->Expire(task->Id());
    return true;
}

void Scheduler::Lib::StandardTaskScheduler::Notify()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    NotifyLocked(lock);
}

void Scheduler::Lib::StandardTaskScheduler::Notify(
    TaskPtr& task,
    TaskState state)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    switch (state)
    {
        case TaskState::ACTIVE:
        {
            assert(m_pending.count(task->Id()) == 0);
            m_active.insert(task->Id());
            Console(std::cout) << "Task '" << task->Id()
                << "' moving to ACTIVE state\n";
            task->SetState(TaskState::ACTIVE);
            break;
        }
        case TaskState::SUCCESS:
        {
            assert(m_pending.count(task->Id()) == 0);
            assert(m_active.count(task->Id()) > 0);
            m_active.erase(task->Id());
            Console(std::cout) << "Task '" << task->Id()
                << "' moving to SUCCESS state\n";
            task->SetState(TaskState::SUCCESS);
            break;
        }
        case TaskState::FAILED:
        {
            assert(m_pending.count(task->Id()) == 0);
            assert(m_active.count(task->Id()) > 0);
            m_active.erase(task->Id());
            Console(std::cout) << "Task '" << task->Id()
                << "' moving to FAILURE state\n";
            task->Fail();
            break;
        }
        default:
            assert(!"Unhandled TaskState state");
    }

#ifdef SCHEDULER_DEBUGGING
    Console() << "Notifying for: " << task->Id() << '\n';
#endif  // SCHEDULER_DEBUGGING

    if (m_waiting) NotifyLocked(lock);

#ifdef SCHEDULER_DEBUGGING
    Console() << "Notify complete for: " << task->Id() << '\n';
#endif  // SCHEDULER_DEBUGGING
}

void Scheduler::Lib::StandardTaskScheduler::NotifyLocked(
    std::unique_lock<std::mutex>& lock)
{
    assert(lock.owns_lock());
    m_notify = true;
    if (!m_waiting) return;
    m_cond.notify_all();
}

bool Scheduler::Lib::StandardTaskScheduler::ProcessActiveTasks()
{
    if (m_active.empty()) return true;

    std::set<UUID> active;
    for (const UUID& uuid : m_active)
    {
        Error error = E_FAILURE;

        TaskPtr task;
        if ((error = m_manager->GetTask(uuid, task)) != E_SUCCESS)
        {
            assert(m_pending.count(uuid) == 0);
            Console(std::cout) << "Unknown active task: " << uuid << "(" << error
                << ")\n";
            continue;
        }

        assert(task->IsValid());

        if (task->IsComplete())
        {
            Console(std::cout) << "Completed task: " << task->Id() << '\n';
            m_manager->Finalize(task);
            continue;
        }
        if (task->IsExpired())
        {
            assert(!task->IsPremature());
            if (!HandleExpiredTask(task)) return false;
            continue;
        }
        active.insert(task->Id());
    }
    active.swap(m_active);
    return true;
}

bool Scheduler::Lib::StandardTaskScheduler::ProcessPendingQueue()
{
    if (m_queue.empty()) return true;

    Error error = E_FAILURE;
    UUID uuid = std::move(m_queue.front());
    m_queue.pop_front();

    TaskPtr task;
    if ((error = m_manager->GetTask(uuid, task)) != E_SUCCESS)
    {
        assert(m_active.count(uuid) == 0);
        assert(m_pending.count(uuid) == 0);

        Console(std::cout) << "Unknown queued task: " << uuid
            << "(" << error << ")\n";
        return true;
    }

    assert(task != nullptr);
    assert(task->IsValid());
    assert(m_active.count(task->Id()) == 0);

    if (task->IsExpired())
    {
        Console(std::cout) << "Task '" << task->Id()
            << "' expired while in queue\n";
        m_manager->Expire(task->Id());
        return true;
    }
    if (task->IsPremature())
    {
        m_premature.emplace(task->Id(), task->After());
        return true;
    }

    m_pending.insert(task->Id());
    task->SetState(TaskState::PENDING);
    return true;
}

bool Scheduler::Lib::StandardTaskScheduler::ProcessPendingTasks()
{
    if (m_pending.empty()) return false;

    bool failed = false;

    std::set<UUID> pending;
    for (const UUID& uuid : m_pending)
    {
        Error error = E_FAILURE;

#ifdef SCHEDULER_DEBUGGING
        Console(std::cout) << "Processing pending task: " << uuid << '\n';
#endif  // SCHEDULER_DEBUGGING

        TaskPtr task;
        if ((error = m_manager->GetTask(uuid, task)) != E_SUCCESS)
        {
            assert(m_active.count(uuid) == 0);
            Console(std::cout) << "Unknown pending task: " << uuid
                << "(" << error << ")\n";
            continue;
        }

        assert(task->IsValid());
        assert(!task->IsComplete());
        assert(!task->IsActive());
        assert(task->GetState() == TaskState::PENDING);

        if (task->IsExpired())
        {
            assert(pending.count(task->Id()) == 0);
            if (!HandleExpiredTask(task)) return false;
            continue;
        }
        if (task->IsPremature())
        {
            // This could happen that a task is retried and given an interval which
            // has not yet occurred. If this is the case, we add it to the premature
            // list.
            if (m_premature.find(task->Id()) == m_premature.end())
                m_premature.emplace(task->Id(), task->After());

            pending.insert(task->Id());
            continue;
        }
        if (!task->HasDependencies())
        {
            assert(!task->IsExpired());
            assert(!task->IsPremature());

#ifdef SCHEDULER_DEBUGGING
            Console(std::cout) << "Processing task with no dependencies: "
                << task->Id() << '\n';
#endif  // SCHEDULER_DEBUGGING

            if (!HandleTask(task)) return false;
            continue;
        }

        bool ready = true;

        for (const TaskPtr& dep : task->GetDependencies())
        {
            if (dep->GetState() == TaskState::SUCCESS) continue;
            if (dep->IsPremature()) ready = false;
            else if (dep->GetState() == TaskState::FAILED)
            {
                ready = false;
                Console(std::cout) << "Failing task '" << task->Id()
                    << "' due to failed dependency '" << dep->Id() << "'\n";
                task->Fail();
                failed = true;
                break;
            }
            else if (dep->IsExpired())
            {
                ready = false;
                Console(std::cout) << "Failing task '" << task->Id()
                    << "' due to expired dependency '" << dep->Id() << "'\n";
                task->Fail();
                failed = true;
                break;
            }
            else if (dep->GetState() == TaskState::NEW)
            {
                ready = false;
                if (IsTimedOut(task))
                {
                    Console(std::cout) << "Failing task '" << task->Id()
                        << "' due to time out on dependency\n";
                    task->Fail();
                    failed = true;
                    break;
                }
                else
                {
                    if (m_timeouts.find(task->Id()) == m_timeouts.end())
                        m_timeouts.emplace(task->Id(), Clock::now());

                    Console(std::cout) << "Task '" << task->Id()
                        << "' is waiting on a unqueued dependency '" << dep->Id()
                        << "'\n";
                }
            }
            else if (m_pending.count(dep->Id()))
            {
                // assert(dep->GetState() == TaskState::PENDING);
                ready = false;
            }
            else if (m_active.count(dep->Id()))
            {
                // assert(dep->GetState() == TaskState::ACTIVE);
                ready = false;
            }
            if (!ready) break;
        }

        if (ready)
        {
            assert(!task->IsComplete());
            if (!HandleTask(task)) return false;
            continue;
        }

        // Tasks with dependencies
        if (!task->IsComplete()) pending.insert(task->Id());
    }

    // After we determine the remaining pending UUIDs we can swap the stored
    // list for the newly generated one and move on.
    pending.swap(m_pending);
    return failed;
}

void Scheduler::Lib::StandardTaskScheduler::PrunePrematureTasks()
{
    for (auto iter = m_premature.begin(); iter != m_premature.end();)
    {
        if (Clock::now() < iter->second)
            iter = m_premature.erase(iter);
        else
            ++iter;
    }
}

bool Scheduler::Lib::StandardTaskScheduler::RunOnce()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    Error error = E_FAILURE;

    if (m_shutdown)
    {
        assert(m_queue.empty());
        assert(m_active.empty());
        assert(m_pending.empty());

        m_shutdownComplete = true;
        m_cond.notify_all();
        return false;
    }

    // Process tasks
    if (!m_queue.empty())
    {
        if (!ProcessPendingQueue()) return false;
        return true;
    }

    if (!ProcessActiveTasks()) return false;
    if (ProcessPendingTasks()) return true;

    // Wait for new tasks to come in

    assert(!m_waiting);
    m_waiting = true;
    if (!m_notify)
    {
        Clock::duration timeout = std::chrono::milliseconds(-1);

        if (!m_premature.empty())
        {
            Clock::time_point now = Clock::now();
            Clock::time_point lowest = now;

            for (const auto& point : m_premature)
                if (point.second < lowest)
                    lowest = point.second;

            if (lowest < now)
                timeout = std::chrono::seconds(0);
            else
                timeout = (lowest - now);
        }

        if (timeout > std::chrono::milliseconds(0))
            m_cond.wait_for(lock, timeout);
        else if (timeout == std::chrono::milliseconds(-1))
            m_cond.wait(lock);
    }

    // something woke up the scheduler
    // Once shutdown is trigger the Reporter, Executor, and Manager are
    // possibly destroyed or about to be.

    assert(lock.owns_lock());
    assert(m_waiting);
    m_notify = false;
    m_waiting = false;
    PrunePrematureTasks();
    return true;
}

std::shared_ptr<Scheduler::Lib::StandardTaskScheduler>
Scheduler::Lib::StandardTaskScheduler::shared_from_this()
{
    return std::static_pointer_cast<StandardTaskScheduler>(
        TaskScheduler::shared_from_this());
}

void Scheduler::Lib::StandardTaskScheduler::Shutdown(bool wait)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_shutdown) return;

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Scheduler shutdown (wait=" << std::boolalpha
        << wait << ")\n";
#endif  // SCHEDULER_DEBUGGING

    assert(!m_shutdownComplete);

    m_shutdown = true;

    ExecutorPtr executor = std::move(m_executor);
    m_executor.reset();

    TaskManagerPtr manager = std::move(m_manager);
    m_manager.reset();

    ScheduleReporterPtr reporter;
    if(m_reporter) reporter = std::move(m_reporter);
    m_reporter.reset();

    std::deque<UUID> queue = std::move(m_queue);
    m_queue.clear();

    std::set<UUID> active = std::move(m_active);
    m_active.clear();

    std::set<UUID> pending = std::move(m_pending);
    m_pending.clear();

    auto premature = std::move(m_premature);
    m_premature.clear();

    auto timeouts = std::move(m_timeouts);
    m_timeouts.clear();

    if (m_waiting) NotifyLocked(lock);
    lock.unlock();

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Shutting down executor\n";
#endif  // SCHEDULER_DEBUGGING

    executor->Shutdown(wait);

#ifdef SCHEDULER_DEBUGGING
    Console(std::cout) << "Shutting down manager\n";
#endif  // SCHEDULER_DEBUGGING

    manager->Shutdown(wait);
    if (reporter) reporter->Shutdown(wait);

    for (const UUID& id : queue) (void)id;
    for (const UUID& id : active) (void)id;
    for (const UUID& id : pending) (void)id;
    for (const auto& id : premature) (void)id;

    if (!wait) return;

    lock.lock();
    while (!m_shutdownComplete) m_cond.wait(lock);

    if (m_thread.joinable()) m_thread.join();
    executor.reset();
    manager.reset();
    reporter.reset();
}

void Scheduler::Lib::StandardTaskScheduler::Start()
{
    std::shared_ptr<StandardTaskScheduler> self(shared_from_this());
    m_thread = std::thread([ self{std::move(self)} ](){
        self->Run();
    });
}

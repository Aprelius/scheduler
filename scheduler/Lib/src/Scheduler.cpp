#include <Scheduler/Lib/Scheduler.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/TaskRunner.h>

#include <iostream>
#include <assert.h>

void Scheduler::Lib::ScheduleReporter::Shutdown(bool wait)
{ }

Scheduler::Error Scheduler::Lib::TaskScheduler::Create(
    const SchedulerParams& params,
    SchedulerPtr& scheduler)
{
    Error error = E_FAILURE;
    std::shared_ptr<Executor> executor;
    std::shared_ptr<ScheduleReporter> reporter;
    std::shared_ptr<TaskManager> manager;

    if (params.executor)
    {
        executor = params.executor->shared_from_this();
    }
    else
    {
        const ExecutorParams& exeParams = params.executorParams;
        if ((error = Executor::Create(exeParams, executor)) != E_SUCCESS)
            return error;
    }
    if (params.manager)
    {
        manager = params.manager->shared_from_this();
    }
    else
    {
        const TaskManagerParams& managerParams = params.managerParams;
        if ((error = TaskManager::Create(managerParams, manager)) != E_SUCCESS)
            return error;
    }
    if (params.reporter)
    {
        reporter = params.reporter->shared_from_this();
    }

    scheduler.reset(new TaskScheduler(params,
        std::move(reporter),
        std::move(manager),
        std::move(executor)));
    if ((error = scheduler->Initialize()) != E_SUCCESS) return error;

    return E_SUCCESS;
}

Scheduler::Lib::TaskScheduler::TaskScheduler(
    const SchedulerParams& params,
    std::shared_ptr<ScheduleReporter>&& reporter,
    std::shared_ptr<TaskManager>&& manager,
    std::shared_ptr<Executor>&& executor)
    : m_executor(std::move(executor)),
      m_reporter(std::move(reporter)),
      m_manager(std::move(manager))
{ }

Scheduler::Lib::TaskScheduler::~TaskScheduler() { Shutdown(false); }

void Scheduler::Lib::TaskScheduler::Enqueue(ChainPtr& chain)
{
    ChainPtr chainPtr = chain->shared_from_this();
    Enqueue(std::move(chainPtr));
}

void Scheduler::Lib::TaskScheduler::Enqueue(ChainPtr&& chain)
{
    if (!chain->IsValid())
    {
        Console(std::cout) << "Invalid chain '" << chain->ToString(true) << "' enqued to scheduler\n";
        return;
    }

    if (!chain->HasChildren())
    {
        Console(std::cout) << "Chain '" << chain->Id() << "' posted with no children\n";
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    Console(std::cout) << "Enqueue chain: " << chain->Id() << '\n';

    for (TaskPtr& child : chain->GetChildren()) EnqueueLocked(child, lock);
    m_queue.emplace_back(chain->Id());

    m_manager->Add(std::move(chain));
    if (m_waiting) NotifyLocked(lock);
}

void Scheduler::Lib::TaskScheduler::Enqueue(TaskPtr& task)
{
    TaskPtr taskPtr = task->shared_from_this();
    Enqueue(std::move(taskPtr));
}

void Scheduler::Lib::TaskScheduler::Enqueue(TaskPtr&& task)
{
    if (!task->IsValid())
    {
        Console(std::cout) << "Invalid task '" << task->ToString(true) << "' enqued to scheduler\n";
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    EnqueueLocked(std::move(task), lock);
}

void Scheduler::Lib::TaskScheduler::EnqueueLocked(
    TaskPtr& task,
    std::unique_lock<std::mutex>& lock)
{
    TaskPtr taskPtr = task->shared_from_this();
    EnqueueLocked(std::move(taskPtr), lock);
}

void Scheduler::Lib::TaskScheduler::EnqueueLocked(
    TaskPtr&& task,
    std::unique_lock<std::mutex>& lock)
{
    m_queue.emplace_back(task->Id());
    Console(std::cout) << "Enqueue: " << task->Id() << '\n';
    m_manager->Add(std::move(task));
    if (m_waiting) NotifyLocked(lock);
}

Scheduler::Error Scheduler::Lib::TaskScheduler::Initialize()
{
    return E_SUCCESS;
}

bool Scheduler::Lib::TaskScheduler::IsTimedOut(const TaskPtr& task) const
{
    static Clock::duration TASK_TIMEOUT_INTERVAL = std::chrono::seconds(30);

    assert(task->IsValid());
    auto iter = m_timeouts.find(task->Id());
    if (iter == m_timeouts.end()) return false;
    return iter->second + TASK_TIMEOUT_INTERVAL > Clock::now();
}

bool Scheduler::Lib::TaskScheduler::HandleTask(TaskPtr& task)
{
    assert(task != nullptr);
    assert(m_active.count(task->Id()) == 0);
    m_active.insert(task->Id());

    Console(std::cout) << "Enqueuing task '" << task->ToString(true)
        << "' with executor" << '\n';

    std::weak_ptr<TaskScheduler> self(shared_from_this());
    TaskRunnerPtr runner = std::make_shared<TaskRunner>(
        std::move(task),
        std::move(self));

    m_executor->Enqueue(runner);
    return true;
}

bool Scheduler::Lib::TaskScheduler::HandleExpiredTask(TaskPtr& task)
{
    assert(task != nullptr);
    assert(task->IsExpired());

    if (task->IsActive())
    {
        Console(std::cout) << "Task '" << task->Id() << "' expired while running\n";
        // Cancel the task in the executor
        task->SetState(TaskState::CANCELLED);
    }
    else
    {
        Console(std::cout) << "Task '" << task->Id() << "' expired while in queue\n";
    }

    PrunePrematureTasks();
    m_manager->Expire(task->Id());
    return true;
}

void Scheduler::Lib::TaskScheduler::Notify()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    NotifyLocked(lock);
}

void Scheduler::Lib::TaskScheduler::Notify(
    const UUID& id,
    TaskState state)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (state == TaskState::ACTIVE)
    {
        assert(m_pending.count(id) == 0);
        m_active.insert(id);
        Console(std::cout) << "Task '" << id << "' moving to active state\n";
    }

    if (m_waiting) NotifyLocked(lock);
}

void Scheduler::Lib::TaskScheduler::NotifyLocked(
    std::unique_lock<std::mutex>& lock)
{
    assert(lock.owns_lock());
    if (!m_waiting) return;
    m_cond.notify_all();
}

bool Scheduler::Lib::TaskScheduler::ProcessActiveTasks()
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

bool Scheduler::Lib::TaskScheduler::ProcessPendingQueue()
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

        Console(std::cout) << "Unknown queued task: " << uuid << "(" << error << ")\n";
        return true;
    }

    assert(task != nullptr);
    assert(task->IsValid());
    assert(m_active.count(task->Id()) == 0);

    if (task->IsExpired())
    {
        Console(std::cout) << "Task '" << task->Id() << "' expired while in queue\n";
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

bool Scheduler::Lib::TaskScheduler::ProcessPendingTasks()
{
    if (m_pending.empty()) return true;

    std::set<UUID> pending;
    for (const UUID& uuid : m_pending)
    {
        Error error = E_FAILURE;
        Console(std::cout) << "Processing pending task: " << uuid << '\n';

        TaskPtr task;
        if ((error = m_manager->GetTask(uuid, task)) != E_SUCCESS)
        {
            assert(m_active.count(uuid) == 0);
            Console(std::cout) << "Unknown pending task: " << uuid << "(" << error
                << ")\n";
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
            pending.insert(task->Id());
            continue;
        }
        if (!task->HasDependencies())
        {
            assert(!task->IsExpired());
            assert(!task->IsPremature());
            Console(std::cout) << "Processing task with no dependencies: "
                << task->Id() << '\n';
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
                break;
            }
            else if (dep->IsExpired())
            {
                ready = false;
                Console(std::cout) << "Failing task '" << task->Id()
                    << "' due to expired dependency '" << dep->Id() << "'\n";
                task->Fail();
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
    return true;
}

void Scheduler::Lib::TaskScheduler::PrunePrematureTasks()
{
    for (auto iter = m_premature.begin(); iter != m_premature.end(); ++iter)
        if (Clock::now() > iter->second) iter = m_premature.erase(iter);
}

bool Scheduler::Lib::TaskScheduler::RunOnce()
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
    if (!ProcessPendingTasks()) return false;

    // Wait for new tasks to come in
    PrunePrematureTasks();

    assert(!m_waiting);
    m_waiting = true;
    m_cond.wait(lock);

    // something woke up the scheduler
    // Once shutdown is trigger the Reporter, Executor, and Manager are
    // possibly destroyed or about to be.

    assert(lock.owns_lock());
    assert(m_waiting);
    m_waiting = false;
    return true;
}

void Scheduler::Lib::TaskScheduler::Shutdown(bool wait)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_shutdown) return;
    Console(std::cout) << "Scheduler shutdown (wait=" << std::boolalpha
        << wait << ")\n";
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

    Console(std::cout) << "Shutting down executor\n";
    executor->Shutdown(wait);
    Console(std::cout) << "Shutting down manager\n";
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

void Scheduler::Lib::TaskScheduler::Start()
{
    std::shared_ptr<TaskScheduler> self(shared_from_this());
    m_thread = std::thread([ self{std::move(self)} ](){
        self->Run();
    });
}
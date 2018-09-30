#include <Scheduler/Lib/ThreadPoolWorker.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/ThreadPoolExecutor.h>
#include <iostream>
#include <assert.h>

// Uncomment to spam yourself with debugging logging.
#define THREAD_POOL_DEBUGGING 1

Scheduler::Lib::ThreadPoolWorker::ThreadPoolWorker(
    std::weak_ptr<ThreadPoolExecutor>&& executor)
    : m_executor(std::move(executor)),
      m_threadId(std::thread::id())
{ }

Scheduler::Lib::ThreadPoolWorker::~ThreadPoolWorker() { Shutdown(); }

void Scheduler::Lib::ThreadPoolWorker::Enqueue(TaskRunnerPtr&& task)
{
    std::lock_guard<std::mutex> lock(m_mutex);

#ifdef THREAD_POOL_DEBUGGING
    Console(std::cout) << "Worker enqueued task: " << task->Id() << '\n';
#endif  // THREAD_POOL_DEBUGGING

    assert(m_threadId != std::this_thread::get_id());
    assert(task->IsValid());

    if (m_shutdown) return;
    m_queue.emplace_back(std::move(task));
    if (m_waiting) m_cond.notify_one();
}

std::hash<std::thread::id>::result_type
Scheduler::Lib::ThreadPoolWorker::Id() const
{
    return std::hash<std::thread::id>{}(m_threadId);
}

void Scheduler::Lib::ThreadPoolWorker::Run()
{
    assert(m_threadId == std::thread::id());

#ifdef THREAD_POOL_DEBUGGING
    Console() << "Worker started\n";
#endif  // THREAD_POOL_DEBUGGING

    m_threadId = std::this_thread::get_id();
    while (RunOnce());

#ifdef THREAD_POOL_DEBUGGING
    Console() << "Worker stopped\n";
#endif  // THREAD_POOL_DEBUGGING
}

bool Scheduler::Lib::ThreadPoolWorker::RunOnce()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_threadId == std::this_thread::get_id());

    if (m_shutdown)
    {
        assert(m_queue.empty());
        m_shutdownComplete = true;
        m_cond.notify_all();
        return false;
    }

    assert(!m_waiting);
    if (m_queue.empty())
    {
        m_waiting = true;
        m_cond.notify_all();
        m_cond.wait(lock);

        assert(m_waiting);
        m_waiting = false;
    }
    if (m_queue.empty()) return true;

    assert(lock.owns_lock());
    TaskRunnerPtr task = std::move(m_queue.front());
    m_queue.pop_front();

    lock.unlock();
    task->Run();
    return true;
}

void Scheduler::Lib::ThreadPoolWorker::Shutdown(bool wait)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_shutdown) return;

#ifdef THREAD_POOL_DEBUGGING
    Console() << "Worker shutdown: " << std::boolalpha << wait << '\n';
#endif  // THREAD_POOL_DEBUGGING

    assert(m_threadId != std::this_thread::get_id());

    m_shutdown = true;

    std::deque<TaskRunnerPtr> queue = std::move(m_queue);
    m_queue.clear();

    lock.unlock();
    m_cond.notify_all();

    // Maybe add a Cancel() call on the task?

    if (!wait) return;

    lock.lock();
    while (!m_shutdownComplete) m_cond.wait(lock);
    if (m_thread.joinable()) m_thread.join();
}

void Scheduler::Lib::ThreadPoolWorker::Start()
{
    assert(m_threadId == std::thread::id());
    m_thread = std::thread([&]{ Run(); });
}

void Scheduler::Lib::ThreadPoolWorker::Wait() const
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_waiting || m_shutdown) return;
    while(!m_waiting && !m_shutdown) m_cond.wait(lock);
}

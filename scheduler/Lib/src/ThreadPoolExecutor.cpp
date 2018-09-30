#include <Scheduler/Lib/ThreadPoolExecutor.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/TaskRunner.h>
#include <Scheduler/Lib/ThreadPoolWorker.h>
#include <Scheduler/Lib/UUID.h>
#include <iostream>

// Uncomment to spam yourself with debugging logging.
#define THREAD_POOL_DEBUGGING 1

Scheduler::Lib::ThreadPoolExecutor::ThreadPoolExecutor(
    const ExecutorParams& params)
    : m_params(params)
{ }

Scheduler::Lib::ThreadPoolExecutor::~ThreadPoolExecutor()
{
    Shutdown(true);
}

Scheduler::Error Scheduler::Lib::ThreadPoolExecutor::Cancel(const UUID& id)
{
    return E_SUCCESS;
}

void Scheduler::Lib::ThreadPoolExecutor::Enqueue(TaskRunnerPtr& task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    unsigned concurrency = m_params.concurrency;

    if (m_shutdown)
    {
        Console(std::cout) << "Task '" << task->Id()
            << "' enqueued after shutdown\n";
        return;
    }

#ifdef THREAD_POOL_DEBUGGING
    Console(std::cout) << "Executor accepted task: " << task->Id() << '\n';
#endif  // THREAD_POOL_DEBUGGING

    TaskRunnerPtr taskPtr = task->shared_from_this();
    size_t hash = std::hash<UUID>{}(taskPtr->Id());
    m_workers[hash % concurrency]->Enqueue(std::move(taskPtr));

#ifdef THREAD_POOL_DEBUGGING
    Console(std::cout) << "Task '" << task->Id() << "' enqueued on worker '"
        << (hash % concurrency) << "'\n";
#endif  // THREAD_POOL_DEBUGGING
}

Scheduler::Error Scheduler::Lib::ThreadPoolExecutor::Initialize()
{
    unsigned concurrency = m_params.concurrency;
    m_workers.reserve(concurrency);

    for (unsigned i = 0; i < concurrency; ++i)
    {
        std::weak_ptr<ThreadPoolExecutor> self(shared_from_this());
        WorkerPtr worker(new ThreadPoolWorker(std::move(self)));
        worker->Start();

        m_workers.emplace_back(std::move(worker));
    }

    // Wait for each of the workers to fully startup before we
    // start sending them tasks.
    for (WorkerPtr& worker : m_workers) worker->Wait();
    return E_SUCCESS;
}

std::shared_ptr<Scheduler::Lib::ThreadPoolExecutor>
Scheduler::Lib::ThreadPoolExecutor::shared_from_this()
{
    return std::static_pointer_cast<ThreadPoolExecutor>(
        Executor::shared_from_this());
}

void Scheduler::Lib::ThreadPoolExecutor::Shutdown(bool wait)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_shutdown) return;

    m_shutdown = true;

#ifdef THREAD_POOL_DEBUGGING
    Console(std::cout) << "Executor shutdown: " << std::boolalpha << wait << '\n';
#endif  // THREAD_POOL_DEBUGGING

    std::vector<WorkerPtr> workers = std::move(m_workers);
    m_workers.clear();

    lock.unlock();

#ifdef THREAD_POOL_DEBUGGING
    Console(std::cout) << "Shutting down '" << workers.size() << "' workers\n";
#endif  // THREAD_POOL_DEBUGGING

    for (WorkerPtr& worker : workers) worker->Shutdown(wait);
    workers.clear();
}

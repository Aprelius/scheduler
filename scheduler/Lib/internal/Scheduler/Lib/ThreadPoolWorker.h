#pragma once

#include <Scheduler/Lib/Task.h>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace Scheduler {
namespace Lib {

    class Task;
    class ThreadPoolExecutor;

    class ThreadPoolWorker
    {
        friend class ThreadPoolExecutor;

        ThreadPoolWorker(const ThreadPoolWorker&) = delete;
        ThreadPoolWorker& operator=(const ThreadPoolWorker&) = delete;

    public:
        ThreadPoolWorker(std::weak_ptr<ThreadPoolExecutor>&& executor);
        ~ThreadPoolWorker();

        void Enqueue(std::shared_ptr<Task>&& task);

        std::hash<std::thread::id>::result_type Id() const;

        void Shutdown(bool wait = true);

        // Wait until the worker is ready to accept tasks.
        void Wait() const;

    private:
        void Run();
        bool RunOnce();
        void Start();

        bool m_shutdown = false;
        bool m_shutdownComplete = false;
        bool m_waiting = false;

        std::weak_ptr<ThreadPoolExecutor> m_executor;

        std::deque<TaskPtr> m_queue;
        mutable std::condition_variable m_cond;
        mutable std::mutex m_mutex;

        std::thread::id m_threadId;
        std::thread m_thread;
    };

}  // namespace Lib
}  // namespace Scheduler

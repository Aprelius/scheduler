#pragma once

#include <Scheduler/Lib/Scheduler.h>

#include <Scheduler/Common/Error.h>
#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Executor.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskManager.h>
#include <Scheduler/Lib/UUID.h>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

namespace Scheduler {
namespace Lib {

    class StandardTaskScheduler : public TaskScheduler
    {
        friend TaskRunner;
        friend TaskScheduler;

    public:
        ~StandardTaskScheduler();

        bool IsShutdown() const { return m_shutdownComplete; }

        void Notify();

        void Run() { while (RunOnce()); }
        bool RunOnce();

        std::shared_ptr<StandardTaskScheduler> shared_from_this();

        void Shutdown(bool wait = true);

        void Start();

    protected:

        Error Initialize();

        void NotifyLocked(std::unique_lock<std::mutex>& lock);

        void Notify(TaskPtr& task, TaskState state);

    private:
        StandardTaskScheduler(
            const SchedulerParams& params,
            std::shared_ptr<ScheduleReporter>&& reporter,
            std::shared_ptr<TaskManager>&& manager,
            std::shared_ptr<Executor>&& executor);

        void Enqueue(Task* task) override;
        void Enqueue(Chain* chain) override;

        void EnqueueLocked(TaskPtr&& task, std::unique_lock<std::mutex>& lock);
        void EnqueueLocked(TaskPtr& task, std::unique_lock<std::mutex>& lock);

        bool HandleTask(TaskPtr& task);
        bool HandleExpiredTask(TaskPtr& task);

        bool IsTimedOut(const TaskPtr& task) const;

        bool ProcessActiveTasks();
        bool ProcessPendingQueue();
        bool ProcessPendingTasks();

        void PrunePrematureTasks();

        // Flag indiciating Notify has been called and the next Wait phase on
        // the scheduler should be skipped.
        bool m_notify = false;
        // Scheduler has been requested to shutdown
        bool m_shutdown = false;
        // Scheduler is shutdown
        bool m_shutdownComplete = false;
        // Scheduler is waiting for changes
        bool m_waiting = false;

        std::condition_variable m_cond;
        std::mutex m_mutex;
        std::deque<UUID> m_queue;
        std::thread m_thread;

        // Cache the UUID of tasks which are active on the executor
        std::set<UUID> m_active;
        // Cache the list of tasks which are currently known and pending
        std::set<UUID> m_pending;
        // Cache the list of tasks which are premature and cannot be
        // run until a certain time.
        std::unordered_map<UUID, Clock::time_point> m_premature;
        // Cache the list of tasks which may be timed out due to an
        // unqueued dependency.
        std::unordered_map<UUID, Clock::time_point> m_timeouts;

        std::shared_ptr<Executor> m_executor;
        std::shared_ptr<ScheduleReporter> m_reporter;
        std::shared_ptr<TaskManager> m_manager;
    };

}  // namespace Lib
}  // namespace Scheduler

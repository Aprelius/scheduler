#pragma once

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

    class ScheduleReporter;

    struct SchedulerParams
    {
        /// Param block for the Task executor. It is only needed if a
        /// pointer to an already existing executor is not supplied.
        ExecutorParams executorParams;

        /// Pointer to a supplied Task executor which will handle
        /// executing the individual tasks onces scheduled.
        Executor* executor = nullptr;

        /// Pointer to the Reporter which should be used for notifying
        /// listeners that an event happened.
        ScheduleReporter* reporter = nullptr;

        /// Param block for the Task manager. It is only needed if a
        /// pointer to an already existing manager is not supplied.
        TaskManagerParams managerParams;

        /// Pointer to the TaskManager object which will serve the
        /// scheduler with Metadata about Tasks.
        TaskManager* manager = nullptr;
    };

    class ScheduleReporter :
        public std::enable_shared_from_this<ScheduleReporter>
    {
    public:
        virtual void Shutdown(bool wait = true);
    };

    class TaskScheduler;
    typedef std::shared_ptr<TaskScheduler> SchedulerPtr;
    typedef std::shared_ptr<ScheduleReporter> ScheduleReporterPtr;

    class TaskScheduler : public std::enable_shared_from_this<TaskScheduler>
    {
        friend TaskRunner;

    public:
        static Error Create(const SchedulerParams& params, SchedulerPtr& sch);

        virtual ~TaskScheduler();

        void Enqueue(ChainPtr& chain);

        void Enqueue(ChainPtr&& chain);

        void Enqueue(TaskPtr& task);

        void Enqueue(TaskPtr&& task);

        void Notify();

        void Run() { while (RunOnce()); }
        bool RunOnce();

        void Shutdown(bool wait = true);

        void Start();

    protected:
        TaskScheduler(
            const SchedulerParams& params,
            std::shared_ptr<ScheduleReporter>&& reporter,
            std::shared_ptr<TaskManager>&& manager,
            std::shared_ptr<Executor>&& executor);

        virtual Error Initialize();

        void NotifyLocked(std::unique_lock<std::mutex>& lock);

        void Notify(TaskPtr& task, TaskState state);

    private:
        void EnqueueLocked(TaskPtr&& task, std::unique_lock<std::mutex>& lock);
        void EnqueueLocked(TaskPtr& task, std::unique_lock<std::mutex>& lock);

        bool HandleTask(TaskPtr& task);
        bool HandleExpiredTask(TaskPtr& task);

        bool IsTimedOut(const TaskPtr& task) const;

        bool ProcessActiveTasks();
        bool ProcessPendingQueue();
        bool ProcessPendingTasks();

        void PrunePrematureTasks();

        // Scheduler is wiating for changes
        bool m_waiting = false;
        // Scheduler has been requested to shutdown
        bool m_shutdown = false;
        // Scheduler is shutdown
        bool m_shutdownComplete = false;

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

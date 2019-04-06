#pragma once

#include <Scheduler/Common/Error.h>
#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Executor.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskManager.h>
#include <Scheduler/Lib/UUID.h>
#include <memory>


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

        virtual ~TaskScheduler() { }

        template<typename T, typename
            std::enable_if<
                std::is_base_of<Task, T>::value
                && !std::is_base_of<Chain, T>::value, T>::type* = nullptr>
        void Enqueue(std::shared_ptr<T>& task)
        {
            Enqueue(task.get());
        }

        template<typename T, typename
            std::enable_if<
                std::is_base_of<Chain, T>::value
                && std::is_convertible<T*, Task*>::value, T>::type* = nullptr>
        void Enqueue(std::shared_ptr<T>& chain)
        {
            Enqueue(chain.get());
        }

        virtual bool IsShutdown() const = 0;

        virtual void Notify() = 0;

        void Run() { while (RunOnce()); }

        virtual void Shutdown(bool wait = true) = 0;

        virtual void Start() = 0;

    protected:
        virtual void Enqueue(Task* task) = 0;

        virtual void Enqueue(Chain* chain) = 0;

        virtual void Notify(TaskPtr& task, TaskState state) = 0;

        virtual bool RunOnce() = 0;
    };

}  // namespace Lib
}  // namespace Scheduler

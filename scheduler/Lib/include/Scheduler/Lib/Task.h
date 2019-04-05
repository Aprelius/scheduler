#pragma once

#include <Scheduler/Common/Clock.h>
#include <Scheduler/Lib/UUID.h>
#include <condition_variable>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

namespace Scheduler {
namespace Lib {

    class Chain;
    class Group;
    class TaskRunner;
    class StandardTaskScheduler;

    enum TaskState
    {
        NEW = 0,
        SUCCESS = 1,
        PENDING,
        ACTIVE,
        FAILED,
        CANCELLED,
        SUSPENDED
    };

    const char* TaskStateToStr(TaskState state);

    std::ostream& operator<<(std::ostream& o, TaskState state);

    enum TaskResult
    {
        RESULT_SUCCESS,
        RESULT_FAILURE,
        RESULT_RETRY
    };

    const char* TaskResultToStr(TaskResult result);

    std::ostream& operator<<(std::ostream& o, TaskResult result);

    class Task;
    typedef std::shared_ptr<Task> TaskPtr;

    class Task : public std::enable_shared_from_this<Task>
    {
        friend class TaskRunner;
        friend class StandardTaskScheduler;

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

    public:
        template<typename T>
        using TaskType = typename std::enable_if<std::is_base_of<Task, T>::value, T>::type;

        template<typename T>
        using TaskTypePtr = std::shared_ptr<TaskType<T>>;

        /// Create a task that should not execute after a given time.
        /// It is expected that the time point given be in the future and
        /// creating one in the past will generate a bad task.
        template<typename T, typename... Args>
        static std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        After(const Clock::time_point& point, Args&& ...args)
        {
            return std::shared_ptr<T>(new T(Clock::time_point::max(), point, std::forward<Args>(args)...));
        /// Create a task that executes the given callable object after a
        /// specified time. It is expected that the time point given be in
        /// the future and creating one in the past will generate an invalid
        /// task.
        template<typename Callable, typename... Args,
            typename = decltype(std::declval<Callable&>())>
        static TaskTypePtr<Task> After(
            Callable&& cb,
            const Clock::time_point& point,
            Args&& ...args)
        {
            return std::shared_ptr<Task::Impl<Task, Callable>>(
                new Task::Impl<Task, Callable>(
                    std::move(cb),
                    Clock::time_point::max(),
                    point,
                    std::forward<Args>(args)...));
        }

        /// Create a Task that should not execute before a given time.
        /// It is expected that the time point given be in the future and
        /// creating one in the past will generate a bad task.
        template<typename T, typename... Args>
        static std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Before(const Clock::time_point& point, Args&& ...args)
        /// Create a Task with should execute a given callable object before
        /// the specified time. It is expected that the time point given be
        // in the future and creating one in the past will generate an invalid
        /// task.
        template<typename Callable, typename... Args,
            typename = decltype(std::declval<Callable&>())>
        static TaskTypePtr<Task> Before(
            Callable&& cb,
            const Clock::time_point& point,
            Args&& ...args)
        {
            return std::shared_ptr<Task::Impl<Task, Callable>>(
                new Task::Impl<Task, Callable>(
                    std::move(cb),
                    point,
                    Clock::time_point::max(),
                    std::forward<Args>(args)...));
        }

        /// Create a task that should execute between two given time
        /// points. Both values should follow the rules for the
        /// individual constructors in be in the future or now.
        template<typename T, typename... Args>
        static std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Between(
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args)
        {
            return std::shared_ptr<T>(new T(before, after, std::forward<Args>(args)...));
        }

        /// Create a Task that should execute a given callable object
        /// between the two specified time points. Both values should
        /// follow the rules for the individual constructors and be
        /// in the future or now.
        template<typename Callable, typename... Args,
            typename = decltype(std::declval<Callable&>())>
        static TaskTypePtr<Task> Between(
            Callable&& cb,
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args)
        {
            return std::shared_ptr<Task::Impl<Task, Callable>>(
                new Task::Impl<Task, Callable>(
                    std::move(cb),
                    before,
                    after,
                    std::forward<Args>(args)...));
        }

        /// Create a simple task that has no time boundaries for execution.
        template<typename T, typename... Args>
        static std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Create(Args&& ...args)
        {
            return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
        }

        /// Create a simple Task with a callable object as the body. This
        /// allows for any type of callback to be scheduled as a Task. There
        /// are no time boundaries associated with this task by default
        /// however the parameters maye be appended and passed to the Task
        /// constructor.
        template<typename Callable, typename ...Args,
            typename = decltype(std::declval<Callable&>())>
        static TaskTypePtr<Task> Create(Callable&& cb, Args&& ...args)
        {
            return std::shared_ptr<Task::Impl<Task, Callable>>(
                new Task::Impl<Task, Callable>(
                    std::move(cb),
                    std::forward<Args>(args)...));
        }

        /// The destructor.
        virtual ~Task();

        /// Return the time point for the given time which this task should not
        /// be executed before.
        Clock::time_point After() const { return m_after; }

        /// Return the time point for the given time which this task should be
        /// executed before it is considered expired.
        Clock::time_point Before() const { return m_before; }

        /// Add the given task a dependency that must complete executing before
        /// this task may begin. ANy changes to the dependency list may be
        /// chained however you should always check IsValid before proceeding to
        /// ensure the dependency chain is a valid run-path.
        Task* Depends(Task* task);

        /// Add the given task a dependency that must complete executing before
        /// this task may begin. ANy changes to the dependency list may be
        /// chained however you should always check IsValid before proceeding to
        /// ensure the dependency chain is a valid run-path.
        template<typename T, typename
            std::enable_if<
                std::is_base_of<Task, T>::value
                && !std::is_base_of<Chain, T>::value, T>::type* = nullptr>
        T* Depends(std::shared_ptr<T>& task)
        {
            return Depends(task.get());
        }

        /// Add the given task a dependency that must complete executing before
        /// this task may begin. ANy changes to the dependency list may be
        /// chained however you should always check IsValid before proceeding to
        /// ensure the dependency chain is a valid run-path.
        template<typename T, typename
            std::enable_if<
                std::is_base_of<Chain, T>::value
                && std::is_convertible<T*, Task*>::value, T>::type* = nullptr>
        T* Depends(std::shared_ptr<T>& chain)
        {
            return reinterpret_cast<T*>(Depends(chain.get()));
        }

        /// Retrieve teh state for the task.
        TaskState GetState() const { return m_state; }

        /// Predicate check if the task has dependencies set.
        bool HasDependencies() const { return !m_dependencies.empty(); }

        /// Retrieve the ID for the task.
        const UUID& Id() const { return m_id; }

        /// The name of the class as it should appear in ToString. Useful
        /// for implementing classes which only need to change the instance
        /// name.
        virtual const char* Instance() const { return "Task"; }

        /// Predeicate to check if the task is currently executing. This
        /// could be multiple states, but it if true it always means the
        /// task can no longer be modified.
        bool IsActive() const;

        /// Predicate check if the task has completed. This should not be
        /// used to determine success or failure, just that the task is now
        /// complete.
        bool IsComplete() const;

        /// Check if the task has expired based on the time range
        /// given during construction.
        bool IsExpired() const;

        /// Check if the task is premature and not yet ready to run based
        /// on the given time range during construction.
        bool IsPremature() const;

        virtual bool IsRetryable() const { return false; }

        /// Predicate check if the task is valid. This flag could get set
        /// during construction or anytime a dependency is added if that
        /// dependency would cause the task to never complete.
        bool IsValid() const;

        /// Check if the given task is a required dependency for this
        // task to run.
        bool Requires(const Task* task) const;

        /// Check if the given task is a required dependency for this
        // task to run.
        bool Requires(const TaskPtr& task) const;

        /// Check if the given task is a required dependency for this
        // task to run.
        bool Requires(const UUID& id) const;

        /// Retrieve the Task identifier as a string or with a descriptive
        /// identifier.
        virtual std::string ToString(bool asShort = false) const;

        /// Wait for a tast to complete. The wait will trigger for any state that
        /// sets it to Complete.
        void Wait(bool complete = true) const;

    protected:
        Task();
        Task(const Clock::time_point& after, const Clock::time_point& before);

        bool Requires(const UUID& start, const UUID& parent, const UUID& id) const;

        virtual void Fail();

        virtual TaskResult Run() { return TaskResult::RESULT_FAILURE; };

        void SetValid(bool status);

    private:
        /// Return a vector of the tasks which are depended on by this task
        /// for completion.
        const std::vector<TaskPtr>& GetDependencies() const
        {
            return m_dependencies;
        }

        virtual Clock::duration GetRetryInterval() const
        {
            return std::chrono::seconds(0);
        }

        void SetAfterTime(const Clock::time_point& point);

        void SetState(TaskState state);
        void SetStateLocked(
            TaskState state,
            std::unique_lock<std::mutex>& lock);

        template<typename Base, typename Callable,
            typename CallableType = decltype(std::declval<Callable&>()),
            typename ResultType = typename std::result_of<CallableType()>::type>
        class Impl : public Base
        {
            static_assert(std::is_move_constructible<Callable>::value,
                "Callable must be move-constructible");
            static_assert(std::is_same<ResultType, TaskResult>::value
                || std::is_same<ResultType, void>::value
                || std::is_same<ResultType, bool>::value,
                "Callable return type must be a TaskResult value or void-type");
        public:
            template<typename ...Args>
            Impl(Callable&& cb, Args&& ...args)
                : Task(std::forward<Args>(args)...),
                  m_callback(std::move(cb)) { }
            ~Impl() { }

            TaskResult Run() final override { return _Run(static_cast<ResultType*>(nullptr)); }

        private:

            TaskResult _Run(bool*)
            {
                if (m_callback()) return TaskResult::RESULT_SUCCESS;
                return TaskResult::RESULT_FAILURE;
            }

            TaskResult _Run(void*) { m_callback(); return TaskResult::RESULT_SUCCESS; }

            TaskResult _Run(TaskResult*) { return m_callback(); }

            Callable m_callback;
            bool m_called = false;
        };

        UUID m_id;
        TaskState m_state;

        Clock::time_point m_createdOn;
        Clock::time_point m_before;
        Clock::time_point m_after;
        bool m_valid = true;

        std::vector<TaskPtr> m_dependencies;
        mutable std::condition_variable m_cond;
        mutable std::mutex m_mutex;
    };

    std::ostream& operator<<(std::ostream& o, const Task* task);

    std::ostream& operator<<(std::ostream& o, const TaskPtr& task);

    template<typename T>
    class RetryableTask : public Task
    {
        friend class Task;

    public:
        ~RetryableTask<T>() { }

        bool IsRetryable() const override { return true; }

    protected:
        using Task::Task;

    private:
        virtual Clock::duration GetRetryInterval() const = 0;
    };

    template<typename T>
    class ImmutableTask : public RetryableTask<T>
    {
        friend class Task;

    public:
        ~ImmutableTask<T>() { }

    protected:
        using RetryableTask<T>::RetryableTask;
    };

}  // namespace Lib
}  // nmaespace Scheduler

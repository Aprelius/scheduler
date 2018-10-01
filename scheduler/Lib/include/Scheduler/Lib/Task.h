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
        CANCELLED
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

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        After(const Clock::time_point& point, Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Before(const Clock::time_point& point, Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        Between(
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args);

        template<typename T, typename... Args>
        friend std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
        MakeTask(Args&& ...args);

    public:

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
        inline Task* Depends(TaskPtr& task) { return Depends(task.get()); }

        /// Return a vector of the tasks which are depended on by this task
        /// for completion.
        const std::vector<TaskPtr>& GetDependencies() const
        {
            return m_dependencies;
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
        void Wait() const;

    protected:
        Task();
        Task(const Clock::time_point& after, const Clock::time_point& before);

        bool Requires(const UUID& start, const UUID& parent, const UUID& id) const;

        virtual void Fail();

        virtual TaskResult Run() = 0;

        void SetValid(bool status);

    private:
        void SetState(TaskState state);
        void SetStateLocked(
            TaskState state,
            std::unique_lock<std::mutex>& lock);

        UUID m_id;
        TaskState m_state;

        Clock::time_point m_createdOn;
        Clock::time_point m_before;
        Clock::time_point m_after;
        bool m_valid = true;

        std::vector<TaskPtr> m_dependencies;
        mutable std::condition_variable m_cond, m_wait;
        mutable std::mutex m_mutex, m_waitex;
    };

    std::ostream& operator<<(std::ostream& o, const Task* task);

    std::ostream& operator<<(std::ostream& o, const TaskPtr& task);

    /// Create a task that should not execute after a given time.
    /// It is expected that the time point given be in the future and
    /// creating one in the past will generate a bad task.
    template<typename T, typename... Args>
    std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
    After(const Clock::time_point& point, Args&& ...args)
    {
        return std::shared_ptr<T>(new T(Clock::time_point::max(), point, std::forward<Args>(args)...));
    }

    /// Create a Task that should not execute before a given time.
    /// It is expected that the time point given be in the future and
    /// creating one in the past will generate a bad task.
    template<typename T, typename... Args>
    std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
    Before(const Clock::time_point& point, Args&& ...args)
    {
        return std::shared_ptr<T>(new T(point, Clock::time_point::max(), std::forward<Args>(args)...));
    }

    /// Create a task that should execute between two given time
    /// points. Both values should follow the rules for the
    /// individual constructors in be in the future or now.
    template<typename T, typename... Args>
    std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
    Between(
        const Clock::time_point& after,
        const Clock::time_point& before,
        Args&& ...args)
    {
        return std::shared_ptr<T>(new T(before, after, std::forward<Args>(args)...));
    }

    /// Create a simple task that has no time boundaries for execution.
    template<typename T, typename... Args>
    std::shared_ptr<typename std::enable_if<std::is_base_of<Task, T>::value, T>::type>
    MakeTask(Args&& ...args)
    {
        return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    }

}  // namespace Lib
}  // nmaespace Scheduler

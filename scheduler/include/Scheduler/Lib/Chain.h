#pragma once

#include <Utilities/PackUtils.h>
#include <Utilities/TypeTraits.h>
#include <Scheduler/Lib/Task.h>
#include <algorithm>
#include <iosfwd>
#include <vector>

namespace Scheduler {
namespace Lib {

    class StandardTaskScheduler;

    class Chain;
    typedef std::shared_ptr<Chain> ChainPtr;

    class Chain : public Task
    {
        friend class StandardTaskScheduler;
        friend class Task;

    public:

        ~Chain();

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        virtual Chain* Add(Task* task);

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        virtual Chain* Add(TaskPtr& task) { return Add(task.get()); }

        /// Predeicate check for whether or not the chain has active children
        /// that are linked.
        bool HasChildren() const { return !m_children.empty(); }

        /// The name of the class as it should appear in ToString. Useful
        /// for implementing classes which only need to change the instance
        /// name.
        const char* Instance() const override { return "Chain"; }

        /// Predicate check for if the given task is a child of the chain. No
        /// ordering is implied by the successful return, only that the child
        /// is required by the chain.
        bool IsChild(const Task* task) const;

        /// Predicate check for if the given task is a child of the chain. No
        /// ordering is implied by the successful return, only that the child
        /// is required by the chain.
        bool IsChild(const TaskPtr& task) const;

        /// Predicate check for if the given task is a child of the chain. No
        /// ordering is implied by the successful return, only that the child
        /// is required by the chain.
        bool IsChild(const UUID& id) const;

        /// Predicate check for checking the status of whetehr the task
        /// is still in a modifiable state. This is expected to be overriden
        /// by implementing classes if the behavior should be changed.
        virtual bool IsModifiable() const { return IsComplete(); }

        const std::vector<TaskPtr>& GetChildren() const { return m_children; }

        ChainPtr shared_from_this();

        /// Retrieve the Chain identifier as a string or with a full
        /// descriptive identifier. If asShort is false, the returning string
        /// will also include the children of the chain.
        std::string ToString(bool asShort = false) const override;

    protected:

        std::vector<TaskPtr>& GetChildren() { return m_children; }

        Chain();

        Chain(
            const Clock::time_point& after,
            const Clock::time_point& before);

        template<typename... Args,
            typename = typename std::enable_if<
                Utilities::AreAllConvertible<TaskPtr, Args...>::value>::type>
        Chain(
            const Clock::time_point& after,
            const Clock::time_point& before,
            Args&& ...args)
            : Task(after, before)
        {
            m_children.reserve(Utilities::PackUtils<Args...>::size);
            Utilities::PackUtils<Args...>::ForEach([&](auto& task){
                this->Add(task);
            }, std::forward<Args>(args)...);
        }

        template<typename... Args,
            typename = typename std::enable_if<
                Utilities::AreAllConvertible<TaskPtr, Args...>::value>::type>
        Chain(Args&& ...args)
        {
            m_children.reserve(Utilities::PackUtils<Args...>::size);
            Utilities::PackUtils<Args...>::ForEach([&](auto& task){
                this->Add(task);
            }, std::forward<Args>(args)...);
        }

        TaskResult Run(ResultPtr&) override;

    private:
        std::vector<TaskPtr> m_children;
    };

    std::ostream& operator<<(std::ostream& o, Chain* chain);

    std::ostream& operator<<(std::ostream& o, ChainPtr& chain);

}  // namespace Lib
}  // namespace Scheduler

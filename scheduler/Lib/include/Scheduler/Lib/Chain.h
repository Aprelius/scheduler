#pragma once

#include <Scheduler/Common/PackUtils.h>
#include <Scheduler/Lib/Task.h>
#include <algorithm>
#include <iosfwd>
#include <vector>

#include <iostream>

namespace Scheduler {
namespace Lib {

    class Chain;
    typedef std::shared_ptr<Chain> ChainPtr;

    class Chain : public Task
    {
    public:
        template<typename... Args>
        static ChainPtr Create(Args&& ...args)
        {
            ChainPtr chain(new Chain);
            chain->m_children.reserve(PackUtils<Args...>::size);

            PackUtils<Args...>::ForEach([&](auto& task){
                chain->Add(task);
            }, std::forward<Args>(args)...);

            return chain;
        }

        ~Chain();

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        Chain* Add(Task* task);

        /// Add a linked child task to the chain. If the chain is already
        /// active or complete then the Add will be skipped. It is always a
        /// good idea to verify that a child was successfully added with the
        /// IsChild call.
        Chain* Add(TaskPtr& task) { return Add(task.get()); }

        /// Predeicate check for whether or not the chain has active children
        /// that are linked.
        bool HasChildren() const { return !m_children.empty(); }

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

        ChainPtr shared_from_this();

        /// Retrieve the Chain identifier as a string or with a full
        /// descriptive identifier. If asShort is false, the returning string
        /// will also include the children of the chain.
        std::string ToString(bool asShort = false) const override;

    private:
        Chain();

        std::vector<TaskPtr> m_children;
    };

    std::ostream& operator<<(std::ostream& o, Chain* chain);

    std::ostream& operator<<(std::ostream& o, ChainPtr& chain);

}  // namespace Lib
}  // namespace Scheduler

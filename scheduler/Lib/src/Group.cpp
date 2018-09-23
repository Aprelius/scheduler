#include <Scheduler/Lib/Group.h>

#include <sstream>

Scheduler::Lib::Group::Group() : Chain() { }

Scheduler::Lib::Group::~Group() { }

Scheduler::Lib::Group* Scheduler::Lib::Group::Add(Task* task)
{
    std::vector<TaskPtr>& children = GetChildren();

    if (IsModifiable()) return this;
    TaskPtr taskPtr = task->shared_from_this();
    SetValid(taskPtr->IsValid());
    children.emplace(children.begin(), std::move(taskPtr));
    return this;
}

Scheduler::Lib::GroupPtr Scheduler::Lib::Group::shared_from_this()
{
    return std::static_pointer_cast<Group>(Chain::shared_from_this());
}

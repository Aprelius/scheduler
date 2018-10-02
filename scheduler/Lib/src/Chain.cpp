#include <Scheduler/Lib/Chain.h>

#include <sstream>

Scheduler::Lib::Chain::Chain()
    : Task()
{ }

Scheduler::Lib::Chain::Chain(
    const Clock::time_point& after,
    const Clock::time_point& before)
    : Task(after, before)
{ }

Scheduler::Lib::Chain::~Chain() { }

Scheduler::Lib::Chain* Scheduler::Lib::Chain::Add(Task* task)
{
    if (IsModifiable()) return this;
    TaskPtr taskPtr = task->shared_from_this();
    Depends(taskPtr);
    if (HasChildren()) taskPtr->Depends(m_children.front());
    SetValid(taskPtr->IsValid());
    m_children.emplace(m_children.begin(), std::move(taskPtr));
    return this;
}

bool Scheduler::Lib::Chain::IsChild(const Task* task) const
{
    return IsChild(task->Id());
}

bool Scheduler::Lib::Chain::IsChild(const TaskPtr& task) const
{
    return IsChild(task->Id());
}

bool Scheduler::Lib::Chain::IsChild(const UUID& id) const
{
    for (const TaskPtr& task : m_children)
    {
        if (task->Id() == id) return true;
    }
    return false;
}

Scheduler::Lib::TaskResult Scheduler::Lib::Chain::Run()
{
    /// This should return the state of the chain, SUCCESS or FAILURE.
    return RESULT_SUCCESS;
}

Scheduler::Lib::ChainPtr Scheduler::Lib::Chain::shared_from_this()
{
    return std::static_pointer_cast<Chain>(Task::shared_from_this());
}

std::string Scheduler::Lib::Chain::ToString(bool asShort) const
{
    if (asShort) return Task::ToString(asShort);

    std::ostringstream o;
    o << "<" <<  Instance() << ": " << Id() << " (" << GetState() << ") [";
    for (size_t i = 0; i < m_children.size(); ++i)
    {
        o << m_children[i];
        if (i + 1 < m_children.size()) o << ", ";
    }
    o << "]>";
    return o.str();
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, Chain* chain)
{
    return o << chain->ToString();
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, ChainPtr& chain)
{
    return o << chain->ToString();
}

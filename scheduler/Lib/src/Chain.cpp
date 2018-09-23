#include <Scheduler/Lib/Chain.h>

#include <sstream>

void Scheduler::Lib::Chain::Process(
    ChainPtr& chain,
    std::vector<TaskPtr>& tasks,
    Task* task)
{
    TaskPtr taskPtr = task->shared_from_this();
    Process(chain, tasks, taskPtr);
}

void Scheduler::Lib::Chain::Process(
    ChainPtr& chain,
    std::vector<TaskPtr>& tasks,
    TaskPtr& task)
{
    if (!tasks.empty()) task->Depends(tasks.front());
    tasks.emplace(tasks.begin(), task->shared_from_this());
    if (!task->IsValid()) chain->SetValid(false);
}

Scheduler::Lib::Chain::Chain()
    : Task()
{}

Scheduler::Lib::Chain::~Chain() { }

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

Scheduler::Lib::ChainPtr Scheduler::Lib::Chain::shared_from_this()
{
    return std::static_pointer_cast<Chain>(Task::shared_from_this());
}

std::string Scheduler::Lib::Chain::ToString(bool asShort) const
{
    if (asShort) return Task::ToString(asShort);

    std::ostringstream o;
    o << "<Chain " << Id() << " (" << GetState() << ") [";
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

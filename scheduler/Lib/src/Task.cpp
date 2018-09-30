#include <Scheduler/Lib/Task.h>

#include <iostream>
#include <sstream>
#include <assert.h>

const char* Scheduler::Lib::TaskStateToStr(TaskState state)
{
    if (state == TaskState::ACTIVE) return "ACTIVE";
    if (state == TaskState::CANCELLED) return "CANCELLED";
    if (state == TaskState::FAILED) return "FAILED";
    if (state == TaskState::NEW) return "NEW";
    if (state == TaskState::PENDING) return "PENDING";
    if (state == TaskState::SUCCESS) return "SUCCESS";
    assert(!"Unknown task state");
    return "<Unknown TaskState>";
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, TaskState state)
{
    return o << TaskStateToStr(state);
}

const char* Scheduler::Lib::TaskResultToStr(TaskResult result)
{
    if (result == TaskResult::RESULT_SUCCESS) return "RESULT_SUCCESS";
    if (result == TaskResult::RESULT_FAILURE) return "RESULT_FAILURE";
    if (result == TaskResult::RESULT_RETRY) return "RESULT_RETRY";
    assert(!"Unknown TaskResult value");
    return "<Unknown TaskResult>";
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, TaskResult result)
{
    return o << TaskResultToStr(result);
}

Scheduler::Lib::Task::Task()
    : m_id(true),
      m_state(TaskState::NEW),
      m_createdOn(Clock::now()),
      m_before(Clock::time_point::max()),
      m_after(Clock::time_point::max())
{ }

Scheduler::Lib::Task::Task(
    const Clock::time_point& before,
    const Clock::time_point& after)
    : m_id(true),
      m_state(TaskState::NEW),
      m_createdOn(Clock::now()),
      m_before(before),
      m_after(after)
{ }

Scheduler::Lib::Task::~Task() { }

Scheduler::Lib::Task* Scheduler::Lib::Task::Depends(Task* task)
{
    if (!task) return this;
    if (!m_valid) return this;

    // There's really no better option in the case that the Task is active.
    // Whenever depending on a submitted task it is probably best to check
    // that the Requires() call comes back true for everything.
    if (IsComplete() || IsActive()) return this;

     // Early check in case the task ID is already linked as a dependency
     // this is to prevent duplicates being added.
    if (Requires(task->Id())) return this;

    // Prevent circular dependencies by invalidating this task if
    // a dependency is added which requires it.
    if (task->Requires(Id())) m_valid = false;

    std::shared_ptr<Task> that = task->shared_from_this();
    m_dependencies.emplace_back(std::move(that));
    return this;
}

void Scheduler::Lib::Task::Fail()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (IsComplete()) return;

    SetStateLocked(TaskState::FAILED, lock);
    // We should probably have some way of notifying every task
    // which depends on this one that it has failed.
}

bool Scheduler::Lib::Task::IsActive() const
{
    return m_state == TaskState::ACTIVE;
}

bool Scheduler::Lib::Task::IsComplete() const
{
    return m_state == TaskState::CANCELLED || m_state == TaskState::FAILED
        || m_state == TaskState::SUCCESS;
}

bool Scheduler::Lib::Task::IsExpired() const
{
    if (m_before != Clock::time_point::max())
        return Clock::now() > m_before;
    return false;
}

bool Scheduler::Lib::Task::IsValid() const
{
    if (!m_valid) return false;
    for (const TaskPtr& task : m_dependencies)
    {
        if (!task->IsValid()) return false;
    }
    return true;
}

bool Scheduler::Lib::Task::Requires(const Task* task) const
{
    if (!task) return false;
    return Requires(Id(), Id(), task->Id());
}

bool Scheduler::Lib::Task::Requires(const TaskPtr& task) const
{
    return Requires(Id(), Id(), task->Id());
}

bool Scheduler::Lib::Task::Requires(const UUID& id) const
{
    return Requires(Id(), Id(), id);
}

bool Scheduler::Lib::Task::Requires(
    const UUID& start,
    const UUID& parent,
    const UUID& id) const
{
    for (const TaskPtr& task : m_dependencies)
    {
        if (task->Id() == start) continue;
        if (task->Id() == parent) continue;
        if (task->Id() == id) return true;
        if (task->Requires(parent, Id(), id)) return true;
    }
    return false;
}

bool Scheduler::Lib::Task::IsPremature() const
{
    if (m_after != Clock::time_point::max())
        return Clock::now() < m_after;
    return false;
}

void Scheduler::Lib::Task::SetState(TaskState state)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    SetStateLocked(state, lock);
}

void Scheduler::Lib::Task::SetStateLocked(
    TaskState state,
    std::unique_lock<std::mutex>& lock)
{
    if (!lock.owns_lock()) lock.lock();
    if (IsComplete()) return;
    {
        std::lock_guard<std::mutex> wait(m_waitex);
        m_state = state;
    }
    m_wait.notify_all();
    m_cond.notify_all();
}

void Scheduler::Lib::Task::SetValid(bool status)
{
    // The validity state should not be changed once the task is
    // complete or has begun running.
    assert(!IsComplete());
    assert(!IsActive());
    if (!m_valid) return;
    m_valid = status;
}

std::string Scheduler::Lib::Task::ToString(bool asShort) const
{
    if (asShort) return m_id.ToString();

    std::ostringstream o;
    o << "<" << Instance() << ": " << m_id << " (" << m_state << ")>";
    return o.str();
}

void Scheduler::Lib::Task::Wait() const
{
    std::unique_lock<std::mutex> lock(m_waitex);
    if (IsComplete()) return;

    m_wait.wait(lock, [&]{ return IsComplete(); });
    m_wait.notify_all();
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, const Task* task)
{
    return o << task->ToString();
}

std::ostream& Scheduler::Lib::operator<<(std::ostream& o, const TaskPtr& task)
{
    return o << task->ToString();
}

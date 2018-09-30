#include <Scheduler/Lib/TaskRunner.h>

#include <Scheduler/Lib/Scheduler.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskManager.h>

Scheduler::Lib::TaskRunner::TaskRunner(TaskPtr&& task)
    : m_task(std::move(task))
{ }

Scheduler::Lib::TaskRunner::TaskRunner(
    TaskPtr&& task,
    std::weak_ptr<TaskScheduler>&& scheduler)
    : m_task(std::move(task)),
      m_scheduler(std::move(scheduler))
{ }

Scheduler::Lib::TaskRunner::~TaskRunner() { Release(); }

const Scheduler::Lib::UUID& Scheduler::Lib::TaskRunner::Id() const
{
    return m_task->Id();
}

bool Scheduler::Lib::TaskRunner::IsValid() const { return m_task->IsValid(); }

void Scheduler::Lib::TaskRunner::Run()
{
    std::shared_ptr<TaskScheduler> scheduler = m_scheduler.lock();

    m_task->SetState(TaskState::ACTIVE);
    if (scheduler) scheduler->Notify(m_task->Id(), m_task->GetState());

    m_task->SetState(TaskState::SUCCESS);
    if (scheduler) scheduler->Notify(m_task->Id(), m_task->GetState());
}

void Scheduler::Lib::TaskRunner::Release()
{
    if (!m_task) return;
    m_task.reset();
    m_scheduler.reset();
}

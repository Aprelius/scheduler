#include <Scheduler/Lib/TaskRunner.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Scheduler.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskManager.h>

#include <iostream>
#include <assert.h>

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

    Clock::time_point start = Clock::now();
    TaskResult result = m_task->Run();
    Clock::time_point stop = Clock::now();

    int64_t length = std::chrono::duration_cast<
        std::chrono::milliseconds>(stop - start).count();

    if (result == TaskResult::RESULT_SUCCESS)
    {
        Console(std::cout) << "Task '" << m_task->Id()
            << "' successfully executed in: " << length << "ms\n";
        m_task->SetState(TaskState::SUCCESS);
    }
    else if (result == TaskResult::RESULT_FAILURE)
    {
        Console(std::cout) << "Task '" << m_task->Id()
            << "' failed to execute after: " << length << "ms\n";
        m_task->SetState(TaskState::FAILED);
    }
    assert(result != TaskResult::RESULT_RETRY);
    if (scheduler) scheduler->Notify(
        m_task->Id(),
        m_task->GetState());
}

void Scheduler::Lib::TaskRunner::Release()
{
    if (!m_task) return;
    m_task.reset();
    m_scheduler.reset();
}

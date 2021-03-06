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

    if (scheduler) scheduler->Notify(
        m_task,
        TaskState::ACTIVE);
    else m_task->SetState(TaskState::ACTIVE);

    Clock::time_point start = Clock::now();
    ResultPtr resultPtr;
    TaskResult result = m_task->Run(resultPtr);
    Clock::time_point stop = Clock::now();

    int64_t length = std::chrono::duration_cast<
        std::chrono::milliseconds>(stop - start).count();

    if (result == TaskResult::SUCCESS)
    {
        Console(std::cout) << "Task '" << m_task->Id()
            << "' successfully executed in: " << length << "ms\n";
        if (scheduler) scheduler->Notify(
            m_task,
            TaskState::SUCCESS);
        else m_task->SetState(TaskState::SUCCESS);
    }
    else if (result == TaskResult::FAILURE)
    {
        Console(std::cout) << "Task '" << m_task->Id()
            << "' failed to execute after: " << length << "ms\n";
        if (scheduler) scheduler->Notify(
            m_task,
            TaskState::FAILED);
        else m_task->SetState(TaskState::FAILED);
    }
    else if (result == TaskResult::RETRY)
    {
        Console(std::cout) << "Task '" << m_task->Id()
            << "' retrying to execute after running for: "
            << length << "ms. Next attempt in: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                m_task->GetRetryInterval()).count()
            << "ms\n";
        m_task->SetAfterTime(Clock::now() + m_task->GetRetryInterval());
        if (scheduler) scheduler->Notify(
            m_task,
            TaskState::PENDING);
        else m_task->SetState(TaskState::PENDING);
    }
    else { assert(!"Unknown TaskResult value"); }
}

void Scheduler::Lib::TaskRunner::Release()
{
    if (!m_task) return;
    m_task.reset();
    m_scheduler.reset();
}

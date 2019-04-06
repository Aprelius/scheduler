#include <Scheduler/Lib/Scheduler.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/StandardTaskScheduler.h>
#include <Scheduler/Lib/TaskRunner.h>

#include <iostream>
#include <assert.h>

void Scheduler::Lib::ScheduleReporter::Shutdown(bool wait)
{ }

Scheduler::Error Scheduler::Lib::TaskScheduler::Create(
    const SchedulerParams& params,
    SchedulerPtr& scheduler)
{
    Error error = E_FAILURE;
    std::shared_ptr<Executor> executor;
    std::shared_ptr<ScheduleReporter> reporter;
    std::shared_ptr<TaskManager> manager;

    if (params.executor)
    {
        executor = params.executor->shared_from_this();
    }
    else
    {
        const ExecutorParams& exeParams = params.executorParams;
        if ((error = Executor::Create(exeParams, executor)) != E_SUCCESS)
            return error;
    }
    if (params.manager)
    {
        manager = params.manager->shared_from_this();
    }
    else
    {
        const TaskManagerParams& managerParams = params.managerParams;
        if ((error = TaskManager::Create(managerParams, manager)) != E_SUCCESS)
            return error;
    }
    if (params.reporter)
    {
        reporter = params.reporter->shared_from_this();
    }

    std::shared_ptr<StandardTaskScheduler> impl(
        new StandardTaskScheduler(params,
            std::move(reporter),
            std::move(manager),
            std::move(executor)));
    if ((error = impl->Initialize()) != E_SUCCESS) return error;

    scheduler = std::move(impl);
    return E_SUCCESS;
}

#include <Scheduler/Lib/TaskManager.h>

#include <Scheduler/Lib/MemoryTaskManager.h>

Scheduler::Error Scheduler::Lib::TaskManager::Create(
    const TaskManagerParams& params,
    TaskManagerPtr& manager)
{
    manager.reset(new MemoryTaskManager(params));

    Error error = E_FAILURE;
    if ((error = manager->Initialize()) != E_SUCCESS)
        return error;

    return E_SUCCESS;
}

void Scheduler::Lib::TaskManager::Expire(const UUID& id)
{
    Error error = E_FAILURE;

    TaskPtr task;
    if ((error = GetTask(id, task)) != E_SUCCESS) return;
    Expire(task);
}

void Scheduler::Lib::TaskManager::Finalize(const UUID& id)
{
    Error error = E_FAILURE;

    TaskPtr task;
    if ((error = GetTask(id, task)) != E_SUCCESS) return;
    Finalize(task);
}

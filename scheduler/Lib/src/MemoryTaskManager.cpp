#include <Scheduler/Lib/MemoryTaskManager.h>

#include <Scheduler/Common/Console.h>
#include <iostream>
#include <assert.h>

Scheduler::Lib::MemoryTaskManager::MemoryTaskManager(
    const TaskManagerParams& params)
    : m_params(params)
{ }

Scheduler::Lib::MemoryTaskManager::~MemoryTaskManager() { }

void Scheduler::Lib::MemoryTaskManager::Add(TaskPtr&& task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    Console(std::cout) << "Add: " <<task->Id() << '\n';
    m_tasks.emplace(task->Id(), std::move(task));
}

void Scheduler::Lib::MemoryTaskManager::Expire(const TaskPtr& task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    Console(std::cout) << "Expire: " << task << '\n';
    m_cache.emplace(task->Id(), TaskState::CANCELLED);
    m_tasks.erase(task->Id());
}

void Scheduler::Lib::MemoryTaskManager::Finalize(const TaskPtr& task)
{
    if (task->IsExpired())
    {
        Expire(task);
        return;
    }

    assert(task->IsComplete());
    std::lock_guard<std::mutex> lock(m_mutex);
    Console(std::cout) << "Finalize: " << task << '\n';

    m_cache.emplace(task->Id(), task->GetState());
}

Scheduler::Error Scheduler::Lib::MemoryTaskManager::GetTask(
    const UUID& id,
    TaskPtr& task) const
{
    auto iter = m_tasks.find(id);
    if (iter == m_tasks.end())
    {
        auto cter = m_cache.find(id);
        if (cter == m_cache.end()) return E_NOT_FOUND;

        if (cter->second == TaskState::SUCCESS)
            return E_COMPLETED;
        if (cter->second == TaskState::FAILED)
            return E_FAILURE;
        if (cter->second == TaskState::CANCELLED)
            return E_CANCELLED;

        assert(!"Unexpected cache state");
        return E_NOT_FOUND;
    }

    task = iter->second->shared_from_this();
    return E_SUCCESS;
}

Scheduler::Error Scheduler::Lib::MemoryTaskManager::Initialize()
{
    return E_SUCCESS;
}

std::shared_ptr<Scheduler::Lib::MemoryTaskManager>
Scheduler::Lib::MemoryTaskManager::shared_from_this()
{
    return std::static_pointer_cast<MemoryTaskManager>(
        TaskManager::shared_from_this());
}

void Scheduler::Lib::MemoryTaskManager::Shutdown(bool wait)
{ }

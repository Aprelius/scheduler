#pragma once

#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskManager.h>
#include <map>
#include <mutex>
#include <unordered_map>

namespace Scheduler {
namespace Lib {

    class MemoryTaskManager : public TaskManager
    {
    public:
        MemoryTaskManager(const TaskManagerParams& params);
        ~MemoryTaskManager();

        void Add(TaskPtr&& task) override;

        void Expire(const TaskPtr& task) override;

        void Finalize(const TaskPtr& task) override;

        Error GetTask(const UUID& id, TaskPtr& task) const override;

        std::shared_ptr<MemoryTaskManager> shared_from_this();

        void Shutdown(bool wait = true) override;

    protected:
        Error Initialize() override;

    private:
        TaskManagerParams m_params;

        std::map<UUID, TaskPtr> m_tasks;
        std::mutex m_mutex;

        std::unordered_map<UUID, TaskState> m_cache;
    };

}  // namespace Lib
}  // namespace Scheduler

#pragma once

#include <memory>

namespace Scheduler {
namespace Lib {

    class Task;
    class TaskManager;
    class TaskScheduler;
    class UUID;

    class TaskRunner;
    typedef std::shared_ptr<TaskRunner> TaskRunnerPtr;

    class TaskRunner : public std::enable_shared_from_this<TaskRunner>
    {
    public:
        TaskRunner(
            std::shared_ptr<Task>&& task);
        TaskRunner(
            std::shared_ptr<Task>&& task,
            std::weak_ptr<TaskScheduler>&& m_scheduler);

        ~TaskRunner();

        const UUID& Id() const;

        bool IsValid() const;

        void Release();

        void Run();

    private:
        std::shared_ptr<Task> m_task;
        std::weak_ptr<TaskScheduler> m_scheduler;
    };

}  // namespace Lib
}  // namespace Scheduler

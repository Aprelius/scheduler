#pragma once

#include <Scheduler/Common/Error.h>
#include <Scheduler/Lib/Task.h>
#include <memory>

namespace Scheduler {
namespace Lib {

    struct TaskManagerParams { };

    class TaskManager;
    typedef std::shared_ptr<TaskManager> TaskManagerPtr;

    class TaskManager : public std::enable_shared_from_this<TaskManager>
    {
    public:
        static Error Create(
            const TaskManagerParams& params,
            TaskManagerPtr& manager);

        virtual ~TaskManager() { }

        virtual void Add(TaskPtr&& task) = 0;

        void Expire(const UUID& id);

        virtual void Expire(const TaskPtr& task) = 0;

        void Finalize(const UUID& id);

        virtual void Finalize(const TaskPtr& task) = 0;

        virtual Error GetTask(const UUID& id, TaskPtr& task) const = 0;

        virtual void Shutdown(bool wait = true) = 0;

    protected:
        virtual Error Initialize() = 0;
    };

}  // namespace Lib
}  // namespace Scheduler

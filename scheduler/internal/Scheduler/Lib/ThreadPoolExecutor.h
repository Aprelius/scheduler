#pragma once

#include <Scheduler/Lib/Executor.h>
#include <mutex>
#include <vector>

namespace Scheduler {
namespace Lib {

    class ThreadPoolWorker;

    class ThreadPoolExecutor : public Executor
    {
    public:
        ThreadPoolExecutor(const ExecutorParams& params);
        ~ThreadPoolExecutor();

        Error Cancel(const UUID& id) override;

        void Enqueue(std::shared_ptr<TaskRunner>& task) override;

        std::shared_ptr<ThreadPoolExecutor> shared_from_this();

        void Shutdown(bool wait = true) override;

    protected:
        Error Initialize() override;

    private:
        ExecutorParams m_params;

        bool m_shutdown = false;
        std::mutex m_mutex;

        typedef std::unique_ptr<ThreadPoolWorker> WorkerPtr;
        std::vector<WorkerPtr> m_workers;
    };

}  // namespace Lib
}  // namespace Scheduler

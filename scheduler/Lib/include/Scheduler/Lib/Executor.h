#pragma once

#include <Scheduler/Common/Error.h>
#include <Scheduler/Lib/Task.h>
#include <memory>

namespace Scheduler {
namespace Lib {

    struct ExecutorParams
    {
        static const unsigned DEFAULT_CONCURRENCY;
        unsigned concurrency = DEFAULT_CONCURRENCY;
    };

    class Executor;
    typedef std::shared_ptr<Executor> ExecutorPtr;

    class Executor : public std::enable_shared_from_this<Executor>
    {
        Executor(const Executor&) = delete;
        Executor& operator=(const Executor&) = delete;

    public:
        static Error Create(const ExecutorParams& params, ExecutorPtr& exe);

        virtual ~Executor();

        /// Cancel a pending task. It may not be possible to cancel a task
        /// once processing has begun on some platform or implementations.
        /// Note: ThreadPool does not support cancelling once the task has
        ///       been picked up by a thread an execution has begun.
        virtual Error Cancel(const UUID& id) = 0;

        /// Enqueue a task for execution. Depending on Implementation it
        /// may simply be moved to an executing worker queue tobe executed
        /// in short time.
        virtual void Enqueue(TaskPtr& task) = 0;

        /// Shutdown the executor. Unless the shutdown is coming as a means
        /// of crashing it is advisable to ALWAYS wait for the shutdown to
        /// complete. Some implementations such as the ThreadPool will cause
        /// a crash if threads are not cleaned up properly.
        virtual void Shutdown(bool wait = true) = 0;

    protected:
        Executor() = default;
        virtual Error Initialize() = 0;
    };

}  // namespace Lib
}  // namespace Scheduler

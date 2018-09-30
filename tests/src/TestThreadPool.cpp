#include <gtest/gtest.h>

#include <Scheduler/Lib/Executor.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Lib/TaskRunner.h>

using namespace Scheduler;
using namespace Scheduler::Lib;

TEST(ExecutorInit, StartupAndShutdown)
{
    Error error = E_FAILURE;
    ExecutorParams params;
    params.concurrency = 1;

    ExecutorPtr executor;
    ASSERT_EQ(Executor::Create(params, executor), E_SUCCESS);

    TaskPtr task = MakeTask<Task>();
    {
        TaskPtr t = task->shared_from_this();
        TaskRunnerPtr runner = std::make_shared<TaskRunner>(
            std::move(t));
        executor->Enqueue(runner);
    }

    task->Wait();
    ASSERT_TRUE(task->IsValid());
    ASSERT_TRUE(task->IsComplete());
    executor->Shutdown(true);
}

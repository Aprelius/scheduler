#include <gtest/gtest.h>

#include <Scheduler/Lib/Executor.h>
#include <Scheduler/Lib/Task.h>

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

    executor->Enqueue(task);

    task->Wait();
    ASSERT_TRUE(task->IsValid());
    ASSERT_TRUE(task->IsComplete());
    executor->Shutdown(true);
}

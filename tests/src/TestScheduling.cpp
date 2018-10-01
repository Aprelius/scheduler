#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Scheduler.h>
#include <Scheduler/Lib/Task.h>

using namespace Scheduler;
using namespace Scheduler::Lib;

TEST(SchedularStartup, InitializeAndShutdown)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>();

    scheduler->Enqueue(taskA);
    scheduler->Enqueue(taskB);

    taskB->Wait();
    taskA->Wait();

    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);

    scheduler->Shutdown(true);
    ASSERT_FALSE(scheduler->RunOnce());
}

TEST(Scheduler, DependentTasks)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>();

    Console(std::cout) << "taskA = " << taskA->ToString(true) << '\n';
    Console(std::cout) << "taskB = " << taskB->ToString(true) << '\n';

    taskA->Depends(taskB);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskA->Requires(taskB));

    scheduler->Enqueue(taskA);
    scheduler->Enqueue(taskB);

    taskB->Wait();
    taskA->Wait();

    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);

    scheduler->Shutdown(true);
    ASSERT_FALSE(scheduler->RunOnce());
}

TEST(Scheduler, ProcessChainAndDependents)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>(),
            taskD = MakeTask<Task>();

    ChainPtr chain = MakeTask<Chain>(taskA, taskB, taskC);

    Console(std::cout) << "taskA = " << taskA->ToString(true) << '\n';
    Console(std::cout) << "taskB = " << taskB->ToString(true) << '\n';
    Console(std::cout) << "taskC = " << taskC->ToString(true) << '\n';
    Console(std::cout) << "taskD = " << taskD->ToString(true) << '\n';

    taskD->Depends(chain.get());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(chain->IsValid());
    ASSERT_TRUE(taskD->Requires(chain.get()));

    scheduler->Enqueue(taskD);
    scheduler->Enqueue(chain);

    taskA->Wait();
    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskA complete\n";

    taskB->Wait();
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskB complete\n";

    taskC->Wait();
    ASSERT_EQ(taskC->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskC complete\n";

    chain->Wait();
    ASSERT_EQ(chain->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "chain complete\n";

    taskD->Wait();
    ASSERT_EQ(taskD->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskD complete\n";

    scheduler->Shutdown(true);
    ASSERT_FALSE(scheduler->RunOnce());
}

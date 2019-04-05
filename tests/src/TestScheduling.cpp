#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Group.h>
#include <Scheduler/Lib/Scheduler.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Tests/Tasks.h>

using namespace Scheduler;
using namespace Scheduler::Lib;
using namespace Scheduler::Tests;

TEST(SchedularStartup, InitializeAndShutdown)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>();

    scheduler->Enqueue(taskA);
    scheduler->Enqueue(taskB);

    taskB->Wait();
    taskA->Wait();

    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, DependentTasks)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>();

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
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, ProcessChainAndDependents)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>();

    ChainPtr chain = Task::Create<Chain>(taskA, taskB, taskC);

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
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, ProcessChainAndDependentsWithFinalFailure)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Failure>(),
            taskD = Task::Create<Success>();

    ChainPtr chain = Task::Create<Chain>(taskA, taskB, taskC);

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

    chain->Wait();
    ASSERT_TRUE(chain->IsComplete());
    ASSERT_EQ(TaskState::FAILED, taskC->GetState());
    ASSERT_EQ(TaskState::FAILED, chain->GetState());

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, ProcessChainAndDependentsWithFirstFailure)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Failure>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>();

    ChainPtr chain = Task::Create<Chain>(taskA, taskB, taskC);

    Console(std::cout) << "chain = " << chain->ToString(true) << '\n';
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
    ASSERT_EQ(TaskState::FAILED, taskA->GetState());
    taskB->Wait();
    ASSERT_EQ(TaskState::FAILED, taskB->GetState());
    taskC->Wait();
    ASSERT_EQ(TaskState::FAILED, taskC->GetState());
    chain->Wait();
    ASSERT_TRUE(chain->IsComplete());
    ASSERT_EQ(TaskState::FAILED, chain->GetState());

    taskD->Wait();
    ASSERT_EQ(TaskState::FAILED, taskD->GetState());

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, ProcessGroupAndDependents)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>();

    GroupPtr group = Task::Create<Group>(taskA, taskB, taskC);

    Console(std::cout) << "taskA = " << taskA->ToString(true) << '\n';
    Console(std::cout) << "taskB = " << taskB->ToString(true) << '\n';
    Console(std::cout) << "taskC = " << taskC->ToString(true) << '\n';
    Console(std::cout) << "taskD = " << taskD->ToString(true) << '\n';

    taskD->Depends(group);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(group->IsValid());
    ASSERT_TRUE(taskD->Requires(group.get()));

    scheduler->Enqueue(taskD);
    scheduler->Enqueue(group);

    taskA->Wait();
    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskA complete\n";

    taskB->Wait();
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskB complete\n";

    taskC->Wait();
    ASSERT_EQ(taskC->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskC complete\n";

    group->Wait();
    ASSERT_EQ(group->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "group complete\n";

    taskD->Wait();
    ASSERT_EQ(taskD->GetState(), TaskState::SUCCESS);
    Console(std::cout) << "taskD complete\n";

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, ProcessGroupAndDependentsWithFirstFailure)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Failure>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>();

    GroupPtr group = Task::Create<Group>(taskA, taskB, taskC);

    Console(std::cout) << "group = " << group->ToString(true) << '\n';
    Console(std::cout) << "taskA = " << taskA->ToString(true) << '\n';
    Console(std::cout) << "taskB = " << taskB->ToString(true) << '\n';
    Console(std::cout) << "taskC = " << taskC->ToString(true) << '\n';
    Console(std::cout) << "taskD = " << taskD->ToString(true) << '\n';

    taskD->Depends(group.get());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(group->IsValid());
    ASSERT_TRUE(taskD->Requires(group));

    scheduler->Enqueue(taskD);
    scheduler->Enqueue(group);

    taskA->Wait();
    ASSERT_EQ(TaskState::FAILED, taskA->GetState());
    taskB->Wait();
    taskC->Wait();
    group->Wait();
    ASSERT_TRUE(group->IsComplete());
    ASSERT_EQ(TaskState::FAILED, group->GetState());

    taskD->Wait();
    ASSERT_EQ(TaskState::FAILED, taskD->GetState());

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}


TEST(Scheduler, BasicRetries)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    TaskPtr taskA = Task::Create<Retry>(),
            taskB = Task::Create<Success>();

    Console(std::cout) << "taskA = " << taskA->ToString(true) << '\n';
    Console(std::cout) << "taskB = " << taskB->ToString(true) << '\n';

    taskB->Depends(taskA);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->Requires(taskA));

    scheduler->Enqueue(taskB);
    scheduler->Enqueue(taskA);

    taskA->Wait(false);
    ASSERT_TRUE(taskA->GetState() == TaskState::PENDING
            || taskA->GetState() == TaskState::ACTIVE);

    taskA->Wait();
    ASSERT_EQ(taskA->GetState(), TaskState::SUCCESS);

    taskB->Wait();
    ASSERT_EQ(taskB->GetState(), TaskState::SUCCESS);

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

TEST(Scheduler, TasksWithLambdas)
{
    SchedulerParams params;
    params.executorParams.concurrency = 2;
    SchedulerPtr scheduler;
    ASSERT_EQ(TaskScheduler::Create(params, scheduler), E_SUCCESS);
    scheduler->Start();

    int value = 0;
    TaskPtr taskA = Task::Create([&]() {
        value = 2;
    });

    ASSERT_TRUE(taskA->IsValid());

    TaskPtr taskB = Task::Create([&]() -> TaskResult {
        value *= 2;
        return TaskResult::RESULT_SUCCESS;
    });

    ASSERT_TRUE(taskB->IsValid());
    taskB->Requires(taskA);
    ASSERT_TRUE(taskB->Depends(taskA));

    TaskPtr taskC = Task::Create([&]() -> bool {
        value /= 4;
        return true;
    });

    ASSERT_TRUE(taskC->IsValid());
    taskC->Requires(taskB);
    ASSERT_TRUE(taskC->Depends(taskB));

    scheduler->Enqueue(taskA);
    scheduler->Enqueue(taskB);
    scheduler->Enqueue(taskC);

    taskC->Wait();
    ASSERT_EQ(value, 1);

    scheduler->Shutdown(true);
    ASSERT_TRUE(scheduler->IsShutdown());
}

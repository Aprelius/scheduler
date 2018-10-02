#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Tests/ClockUtils.h>
#include <Scheduler/Tests/Tasks.h>

using namespace Scheduler;
using namespace Scheduler::Lib;
using namespace Scheduler::Tests;
using std::chrono::seconds;

TEST(ChainingTasks, ByContruction)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

    ChainPtr chain = Task::Create<Chain>(taskA, taskB, taskC);

    // Chain[taskA -> taskB -> taskC]

    ASSERT_TRUE(chain->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    ASSERT_TRUE(taskC->Requires(taskB));
    ASSERT_TRUE(taskB->Requires(taskA));

    ASSERT_TRUE(chain->IsChild(taskA));
    ASSERT_TRUE(chain->IsChild(taskB));
    ASSERT_TRUE(chain->IsChild(taskC));
    Console(std::cout) << chain << '\n';
}

TEST(ChainingTasks, CircularInvalidBeforeConstruction)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>();

    // taskA
    //      \
    //      taskB -> taskC
    //      /
    // taskD

    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());
    ASSERT_TRUE(taskD->IsValid());

    // Good
    taskA->Depends(taskB);
    ASSERT_TRUE(taskA->Requires(taskB));
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());

    // Good
    taskB->Depends(taskC);
    ASSERT_TRUE(taskB->Requires(taskC));
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    // Good
    taskD->Depends(taskB);
    ASSERT_TRUE(taskD->Requires(taskB));
    ASSERT_TRUE(taskD->IsValid());
    ASSERT_TRUE(taskB->IsValid());

    // Circular dependency on taskA
    // Circular: taskA -> taskB ->taskC
    taskC->Depends(taskA);
    ASSERT_TRUE(taskC->Requires(taskA));
    ASSERT_FALSE(taskA->IsValid());
    ASSERT_FALSE(taskC->IsValid());

    Console(std::cout) << "TaskA = " << taskA << '\n';
    Console(std::cout) << "TaskB = " << taskB << '\n';
    Console(std::cout) << "TaskC = " << taskC << '\n';
    Console(std::cout) << "TaskD = " << taskD << '\n';

    // taskA and taskC are already invalid by the time the chain
    // is constructed.
    ChainPtr chain = Task::Create<Chain>(taskA, taskC, taskD);
    ASSERT_FALSE(chain->IsValid());
    Console(std::cout) << chain << '\n';
}

TEST(ChainingTasks, InvalidMultiLevelCircular)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>(),
            taskD = Task::Create<Success>(),
            taskE = Task::Create<Success>();

    Console(std::cout) << "TaskA = " << taskA << '\n';
    Console(std::cout) << "TaskB = " << taskB << '\n';
    Console(std::cout) << "TaskC = " << taskC << '\n';
    Console(std::cout) << "TaskD = " << taskD << '\n';
    Console(std::cout) << "TaskE = " << taskE << '\n';

    taskA->Depends(taskD);
    ASSERT_TRUE(taskA->Requires(taskD));

    ChainPtr chain = Task::Create<Chain>(taskA, taskB, taskC, taskD);
    ASSERT_FALSE(chain->IsValid());
    Console(std::cout) << chain << '\n';
}

TEST(ChainingTasks, InvalidByConstruction)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

    taskB->Depends(taskA);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());

    ChainPtr chain = Task::Create<Chain>(taskB, taskA, taskC);
    ASSERT_FALSE(chain->IsValid());
    Console(std::cout) << chain << '\n';
}

TEST(ChainingTaskConstructors, AfterContruction)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

    ChainPtr chain = Task::After<Chain>(
        Future(seconds(10)),
        taskA, taskB, taskC);

    ASSERT_EQ(chain->GetState(), TaskState::NEW);
    ASSERT_TRUE(chain->IsPremature());
    ASSERT_FALSE(chain->IsExpired());

    // Chain[taskA -> taskB -> taskC]

    ASSERT_TRUE(chain->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    ASSERT_TRUE(taskC->Requires(taskB));
    ASSERT_TRUE(taskB->Requires(taskA));

    ASSERT_TRUE(chain->IsChild(taskA));
    ASSERT_TRUE(chain->IsChild(taskB));
    ASSERT_TRUE(chain->IsChild(taskC));
    Console(std::cout) << chain << '\n';
}

TEST(ChainingTaskConstructors, BeforeContruction)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

    ChainPtr chain = Task::Before<Chain>(
        Future(seconds(10)),
        taskA, taskB, taskC);

    ASSERT_EQ(chain->GetState(), TaskState::NEW);
    ASSERT_FALSE(chain->IsPremature());
    ASSERT_FALSE(chain->IsExpired());

    // Chain[taskA -> taskB -> taskC]

    ASSERT_TRUE(chain->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    ASSERT_TRUE(taskC->Requires(taskB));
    ASSERT_TRUE(taskB->Requires(taskA));

    ASSERT_TRUE(chain->IsChild(taskA));
    ASSERT_TRUE(chain->IsChild(taskB));
    ASSERT_TRUE(chain->IsChild(taskC));
    Console(std::cout) << chain << '\n';
}

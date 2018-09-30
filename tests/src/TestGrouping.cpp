#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Group.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Tests/ClockUtils.h>

using namespace Scheduler;
using namespace Scheduler::Lib;
using namespace Scheduler::Tests;
using std::chrono::seconds;

TEST(GroupingTasks, ByContruction)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>();

    GroupPtr group = MakeTask<Group>(taskA, taskB, taskC);

    // Group[taskA -> taskB -> taskC]

    ASSERT_TRUE(group->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    // Groups don't link task dependencies
    ASSERT_FALSE(taskC->Requires(taskB));
    ASSERT_FALSE(taskB->Requires(taskA));

    ASSERT_TRUE(group->IsChild(taskA));
    ASSERT_TRUE(group->IsChild(taskB));
    ASSERT_TRUE(group->IsChild(taskC));
    Console(std::cout) << group << '\n';
}

TEST(GroupingTasks, CircularInvalidBeforeConstruction)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>(),
            taskD = MakeTask<Task>();

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

    // taskA and taskC are already invalid by the time the group
    // is constructed.
    GroupPtr group = MakeTask<Group>(taskA, taskC, taskD);
    ASSERT_FALSE(group->IsValid());
    Console(std::cout) << group << '\n';
}

TEST(GroupingTasks, ValidMultiLevelCircular)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>(),
            taskD = MakeTask<Task>(),
            taskE = MakeTask<Task>();

    Console(std::cout) << "TaskA = " << taskA << '\n';
    Console(std::cout) << "TaskB = " << taskB << '\n';
    Console(std::cout) << "TaskC = " << taskC << '\n';
    Console(std::cout) << "TaskD = " << taskD << '\n';
    Console(std::cout) << "TaskE = " << taskE << '\n';

    taskA->Depends(taskD);
    ASSERT_TRUE(taskA->Requires(taskD));

    // This is completely valid because the Group will execute the entire
    // batch of tasks and properly handle the added ordering.

    GroupPtr group = MakeTask<Group>(taskA, taskB, taskC, taskD);
    ASSERT_TRUE(group->IsValid());
    Console(std::cout) << group << '\n';
}

TEST(GroupingTaskConstructors, AfterContruction)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>();

    GroupPtr group = After<Group>(
        Future(seconds(10)),
        taskA, taskB, taskC);

    ASSERT_EQ(group->GetState(), TaskState::NEW);
    ASSERT_TRUE(group->IsPremature());
    ASSERT_FALSE(group->IsExpired());

    // Group[taskA -> taskB -> taskC]

    ASSERT_TRUE(group->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    ASSERT_FALSE(taskC->Requires(taskB));
    ASSERT_FALSE(taskB->Requires(taskA));

    ASSERT_TRUE(group->IsChild(taskA));
    ASSERT_TRUE(group->IsChild(taskB));
    ASSERT_TRUE(group->IsChild(taskC));
    Console(std::cout) << group << '\n';
}

TEST(GroupingTaskConstructors, BeforeContruction)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>();

    GroupPtr group = Before<Group>(
        Future(seconds(10)),
        taskA, taskB, taskC);

    ASSERT_EQ(group->GetState(), TaskState::NEW);
    ASSERT_FALSE(group->IsPremature());
    ASSERT_FALSE(group->IsExpired());

    // Group[taskA -> taskB -> taskC]

    ASSERT_TRUE(group->IsValid());
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskC->IsValid());

    ASSERT_FALSE(taskC->Requires(taskB));
    ASSERT_FALSE(taskB->Requires(taskA));

    ASSERT_TRUE(group->IsChild(taskA));
    ASSERT_TRUE(group->IsChild(taskB));
    ASSERT_TRUE(group->IsChild(taskC));
    Console(std::cout) << group << '\n';
}

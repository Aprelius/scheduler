#include <gtest/gtest.h>

#include <Scheduler/Lib/Task.h>
#include <Scheduler/Tests/ClockUtils.h>
#include <iostream>

using std::chrono::seconds;
using namespace Scheduler;
using namespace Scheduler::Lib;
using namespace Scheduler::Tests;

Clock::time_point Scheduler::Tests::Future(const Clock::duration& length)
{
    return Clock::now() + length;
}

Clock::time_point Scheduler::Tests::Past(const Clock::duration& length)
{
    return Clock::now() - length;
}

TEST(TaskConstruction, CreateSimpleTask)
{
    TaskPtr task = MakeTask<Task>();
    ASSERT_EQ(task->GetState(), TaskState::NEW);
    ASSERT_FALSE(task->IsExpired());
    ASSERT_FALSE(task->IsPremature());
    ASSERT_FALSE(task->HasDependencies());
}

TEST(TaskConstruction, CreateSimpleAfterTask)
{
    TaskPtr taskA = After<Task>(Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = After<Task>(Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBeforeTask)
{
    TaskPtr taskA = Before<Task>(Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Before<Task>(Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_TRUE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBetweenTask)
{
    TaskPtr taskA = Between<Task>(Clock::now(), Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Between<Task>(Future(seconds(10)), Future(seconds(15)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());

    TaskPtr taskC = Between<Task>(Past(seconds(15)), Past(seconds(10)));
    ASSERT_EQ(taskC->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskC->IsPremature());
    ASSERT_TRUE(taskC->IsExpired());
}

TEST(TaskDependencies, SimpleDependencies)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>();

    ASSERT_FALSE(taskA->HasDependencies());
    ASSERT_FALSE(taskB->HasDependencies());
    ASSERT_FALSE(taskC->HasDependencies());

    taskA->Depends(taskB);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());
    ASSERT_TRUE(taskA->Requires(taskB));
    ASSERT_TRUE(taskA->HasDependencies());

    taskB->Depends(taskC);
    ASSERT_TRUE(taskC->IsValid());
    ASSERT_TRUE(taskC->IsValid());
    ASSERT_TRUE(taskB->Requires(taskC));
    ASSERT_TRUE(taskB->HasDependencies());

    ASSERT_TRUE(taskA->Requires(taskC));
}

TEST(TaskDependencies, CircularDependencies)
{
    TaskPtr taskA = MakeTask<Task>(),
            taskB = MakeTask<Task>(),
            taskC = MakeTask<Task>();

    ASSERT_FALSE(taskA->HasDependencies());
    ASSERT_FALSE(taskB->HasDependencies());
    ASSERT_FALSE(taskC->HasDependencies());

    taskA->Depends(taskB);
    taskB->Depends(taskC);
    taskC->Depends(taskA);

    ASSERT_FALSE(taskC->IsValid());
    ASSERT_FALSE(taskB->IsValid());
    ASSERT_FALSE(taskA->IsValid());
}

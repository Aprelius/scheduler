#include <gtest/gtest.h>

#include <Scheduler/Common/Console.h>
#include <Scheduler/Lib/Task.h>
#include <Scheduler/Tests/ClockUtils.h>
#include <Scheduler/Tests/Tasks.h>
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
    TaskPtr task = Task::Create<Success>();
    ASSERT_EQ(task->GetState(), TaskState::NEW);
    ASSERT_FALSE(task->IsExpired());
    ASSERT_FALSE(task->IsPremature());
    ASSERT_FALSE(task->HasDependencies());
}

TEST(TaskConstruction, CreateSimpleAfterTask)
{
    TaskPtr taskA = Task::After<Success>(Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::After<Success>(Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleAfterTask_WithLambdas_WithResult)
{
    bool capture = false;
    TaskPtr taskA = Task::After([&](ResultPtr&){ capture = true; }, Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::After([&](ResultPtr&){ capture = true; }, Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleAfterTask_WithLambdas_WithoutResult)
{
    bool capture = false;
    TaskPtr taskA = Task::After([&](){ capture = true; }, Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::After([&](){ capture = true; }, Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBeforeTask)
{
    TaskPtr taskA = Task::Before<Success>(Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Before<Success>(Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_TRUE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBeforeTask_WithLambdas_WithResult)
{
    bool capture = false;
    TaskPtr taskA = Task::Before([&](ResultPtr&){ capture = true; }, Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Before([&](ResultPtr&){ capture = true; }, Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_TRUE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBeforeTask_WithLambdas_WithoutResult)
{
    bool capture = false;
    TaskPtr taskA = Task::Before([&](){ capture = true; }, Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Before([&](){ capture = true; }, Past(seconds(10)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskB->IsPremature());
    ASSERT_TRUE(taskB->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBetweenTask)
{
    TaskPtr taskA = Task::Between<Success>(Clock::now(), Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Between<Success>(Future(seconds(10)), Future(seconds(15)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());

    TaskPtr taskC = Task::Between<Success>(Past(seconds(15)), Past(seconds(10)));
    ASSERT_EQ(taskC->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskC->IsPremature());
    ASSERT_TRUE(taskC->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBetweenTask_WithLambdas_WithResult)
{
    bool capture = false;
    TaskPtr taskA = Task::Between([&](ResultPtr&){ capture = true; },
        Clock::now(), Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Between([&](ResultPtr&){ capture = true; },
        Future(seconds(10)), Future(seconds(15)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());

    TaskPtr taskC = Task::Between([&](ResultPtr&){ capture = true; },
        Past(seconds(15)), Past(seconds(10)));
    ASSERT_EQ(taskC->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskC->IsPremature());
    ASSERT_TRUE(taskC->IsExpired());
}

TEST(TaskConstruction, CreateSimpleBetweenTask_WithLambdas_WithoutResult)
{
    bool capture = false;
    TaskPtr taskA = Task::Between([&](){ capture = true; },
        Clock::now(), Future(seconds(10)));
    ASSERT_EQ(taskA->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskA->IsPremature());
    ASSERT_FALSE(taskA->IsExpired());

    TaskPtr taskB = Task::Between([&](){ capture = true; },
        Future(seconds(10)), Future(seconds(15)));
    ASSERT_EQ(taskB->GetState(), TaskState::NEW);
    ASSERT_TRUE(taskB->IsPremature());
    ASSERT_FALSE(taskB->IsExpired());

    TaskPtr taskC = Task::Between([&](){ capture = true; },
        Past(seconds(15)), Past(seconds(10)));
    ASSERT_EQ(taskC->GetState(), TaskState::NEW);
    ASSERT_FALSE(taskC->IsPremature());
    ASSERT_TRUE(taskC->IsExpired());
}

TEST(TaskConstruction, TasksWithLambdas_WithResult)
{
    bool value = 0;
    TaskPtr taskA = Task::Create([&](ResultPtr&) {
        value = 1;
    });

    ASSERT_TRUE(taskA->IsValid());

    value = 0;
    TaskPtr taskB = Task::Create([&](ResultPtr&) -> TaskResult {
        value = 1;
        return TaskResult::SUCCESS;
    });

    ASSERT_TRUE(taskB->IsValid());

    value = 0;
    TaskPtr taskC = Task::Create([&](ResultPtr&) -> bool {
        value = 1;
        return true;
    });

    ASSERT_TRUE(taskC->IsValid());
}

TEST(TaskConstruction, TasksWithLambdas_WithoutResult)
{
    bool value = 0;
    TaskPtr taskA = Task::Create([&]() {
        value = 1;
    });

    ASSERT_TRUE(taskA->IsValid());

    value = 0;
    TaskPtr taskB = Task::Create([&]() -> TaskResult {
        value = 1;
        return TaskResult::SUCCESS;
    });

    ASSERT_TRUE(taskB->IsValid());

    value = 0;
    TaskPtr taskC = Task::Create([&]() -> bool {
        value = 1;
        return true;
    });

    ASSERT_TRUE(taskC->IsValid());
}

TEST(TaskConstruction, TasksWithLambdas_WithTask)
{
    bool value = 0;
    TaskPtr taskA = Task::Create([&](Task* task) {
        value = 1;
        Console(std::cout) << "taskA = " << task;
    });

    ASSERT_TRUE(taskA->IsValid());

    value = 0;
    TaskPtr taskB = Task::Create([&](Task* task) -> TaskResult {
        value = 1;
        Console(std::cout) << "taskB = " << task;
        return TaskResult::SUCCESS;
    });

    ASSERT_TRUE(taskB->IsValid());

    value = 0;
    TaskPtr taskC = Task::Create([&](Task* task) -> bool {
        value = 1;
        Console(std::cout) << "taskC = " << task;
        return true;
    });

    ASSERT_TRUE(taskC->IsValid());
}

TEST(TaskConstruction, TasksWithLambdas_WithTaskResult)
{
    bool value = 0;
    TaskPtr taskA = Task::Create([&](Task* task, ResultPtr&) {
        value = 1;
        Console(std::cout) << "taskA = " << task << '\n';
    });

    ASSERT_TRUE(taskA->IsValid());

    value = 0;
    TaskPtr taskB = Task::Create([&](Task* task, ResultPtr&) -> TaskResult {
        value = 1;
        Console(std::cout) << "taskB = " << task << '\n';
        return TaskResult::SUCCESS;
    });

    ASSERT_TRUE(taskB->IsValid());

    value = 0;
    TaskPtr taskC = Task::Create([&](Task* task, ResultPtr&) -> bool {
        value = 1;
        Console(std::cout) << "taskC = " << task << '\n';
        return true;
    });

    ASSERT_TRUE(taskC->IsValid());
}

TEST(TaskDependencies, SimpleDependencies)
{
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

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
    TaskPtr taskA = Task::Create<Success>(),
            taskB = Task::Create<Success>(),
            taskC = Task::Create<Success>();

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

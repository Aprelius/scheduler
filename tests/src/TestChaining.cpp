#include <gtest/gtest.h>

#include <Scheduler/Lib/Chain.h>
#include <Scheduler/Lib/Task.h>

using namespace Scheduler::Lib;

TEST(ChainingTasks, ByContruction)
{
    TaskPtr taskA = Task::Create(),
            taskB = Task::Create(),
            taskC = Task::Create();

    ChainPtr chain = Chain::Create(taskA, taskB, taskC);

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
    std::cout << chain << '\n';
}

TEST(ChainingTasks, CircularInvalidBeforeConstruction)
{
    TaskPtr taskA = Task::Create(),
            taskB = Task::Create(),
            taskC = Task::Create(),
            taskD = Task::Create();

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

    std::cout << "TaskA = " << taskA << '\n';
    std::cout << "TaskB = " << taskB << '\n';
    std::cout << "TaskC = " << taskC << '\n';
    std::cout << "TaskD = " << taskD << '\n';

    // taskA and taskC are already invalid by the time the chain
    // is constructed.
    ChainPtr chain = Chain::Create(taskA, taskC, taskD);
    ASSERT_FALSE(chain->IsValid());
    std::cout << chain << '\n';
}

TEST(ChainingTasks, InvalidMultiLevelCircular)
{
    TaskPtr taskA = Task::Create(),
            taskB = Task::Create(),
            taskC = Task::Create(),
            taskD = Task::Create(),
            taskE = Task::Create();

    std::cout << "TaskA = " << taskA << '\n';
    std::cout << "TaskB = " << taskB << '\n';
    std::cout << "TaskC = " << taskC << '\n';
    std::cout << "TaskD = " << taskD << '\n';
    std::cout << "TaskE = " << taskE << '\n';

    taskA->Depends(taskD);
    ASSERT_TRUE(taskA->Requires(taskD));

    ChainPtr chain = Chain::Create(taskA, taskB, taskC, taskD);
    ASSERT_FALSE(chain->IsValid());
    std::cout << chain << '\n';
}

TEST(ChainingTasks, InvalidByConstruction)
{
    TaskPtr taskA = Task::Create(),
            taskB = Task::Create(),
            taskC = Task::Create();

    taskB->Depends(taskA);
    ASSERT_TRUE(taskA->IsValid());
    ASSERT_TRUE(taskB->IsValid());

    ChainPtr chain = Chain::Create(taskB, taskA, taskC);
    ASSERT_FALSE(chain->IsValid());
    std::cout << chain << '\n';
}

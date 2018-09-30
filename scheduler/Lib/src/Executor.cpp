#include <Scheduler/Lib/Executor.h>

#include <Scheduler/Lib/ThreadPoolExecutor.h>
#include <thread>

const unsigned Scheduler::Lib::ExecutorParams::DEFAULT_CONCURRENCY
    = std::thread::hardware_concurrency();

Scheduler::Error Scheduler::Lib::Executor::Create(
    const ExecutorParams& params,
    ExecutorPtr& executor)
{
    executor.reset(new ThreadPoolExecutor(params));

    Error error = E_FAILURE;
    if ((error = executor->Initialize()) != E_SUCCESS)
        return error;

    return E_SUCCESS;
}

Scheduler::Lib::Executor::~Executor() { }

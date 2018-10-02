#pragma once

#include <Scheduler/Lib/Task.h>

namespace Scheduler {
namespace Tests {

    class Success : public Lib::RetryableTask<Success>
    {
    public:
        ~Success() { }

    protected:
        using Lib::RetryableTask<Success>::RetryableTask;

    private:
        Lib::TaskResult Run() { return Lib::RESULT_SUCCESS; }
    };

}  // namespace Tests
}  // namespace Scheduler

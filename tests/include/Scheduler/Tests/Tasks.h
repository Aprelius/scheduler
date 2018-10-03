#pragma once

#include <Scheduler/Lib/Task.h>

namespace Scheduler {
namespace Tests {

    class Failure : public Lib::ImmutableTask<Failure>
    {
    public:
        ~Failure() { }

    protected:
        using Lib::ImmutableTask<Failure>::ImmutableTask;

    private:
        Lib::TaskResult Run() { return Lib::RESULT_FAILURE; }
    };

    class Success : public Lib::ImmutableTask<Success>
    {
    public:
        ~Success() { }

    protected:
        using Lib::ImmutableTask<Success>::ImmutableTask;

    private:
        Lib::TaskResult Run() { return Lib::RESULT_SUCCESS; }
    };

}  // namespace Tests
}  // namespace Scheduler

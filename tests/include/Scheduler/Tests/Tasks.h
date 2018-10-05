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
        Clock::duration GetRetryInterval() const override
        {
            return std::chrono::seconds(0);
        }

        Lib::TaskResult Run() { return Lib::RESULT_FAILURE; }
    };

    class Retry : public Lib::ImmutableTask<Retry>
    {
    public:
        ~Retry() { }

    protected:
        using Lib::ImmutableTask<Retry>::ImmutableTask;

    private:
        Clock::duration GetRetryInterval() const override
        {
            return std::chrono::milliseconds(10);
        }

        Lib::TaskResult Run()
        {
            if (!m_retried)
            {
                m_retried = true;
                return Lib::RESULT_RETRY;
            }
            return Lib::RESULT_SUCCESS;
        }

        bool m_retried = false;
    };

    class Success : public Lib::ImmutableTask<Success>
    {
    public:
        ~Success() { }

    protected:
        using Lib::ImmutableTask<Success>::ImmutableTask;

    private:
        Clock::duration GetRetryInterval() const override
        {
            return std::chrono::seconds(0);
        }

        Lib::TaskResult Run() { return Lib::RESULT_SUCCESS; }
    };

}  // namespace Tests
}  // namespace Scheduler

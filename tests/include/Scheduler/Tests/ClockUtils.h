#pragma once

#include <Scheduler/Common/Clock.h>

namespace Scheduler {
namespace Tests {

    Clock::time_point Future(const Clock::duration& length);

    Clock::time_point Past(const Clock::duration& length);

}  // namespace Tests
}  // namespace Scheduler

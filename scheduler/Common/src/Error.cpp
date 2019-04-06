#include <Scheduler/Common/Error.h>

#include <ostream>

const char* Scheduler::ErrorToStr(Error e)
{
    if (e == E_FAILURE) return "E_FAILURE";
    if (e == E_SUCCESS) return "E_SUCCESS";
    return "<Unknown>";
}

std::ostream& Scheduler::operator<<(
    std::ostream& o,
    Error e)
{
    return o << ErrorToStr(e);
}

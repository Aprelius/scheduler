#include <Scheduler/Common/Error.h>

#include <ostream>
#include <assert.h>

const char* Scheduler::ErrorToStr(Error e)
{
    if (e == E_FAILURE) return "E_FAILURE";
    if (e == E_SUCCESS) return "E_SUCCESS";
    if (e == E_NOT_FOUND) return "E_NOT_FOUND";
    if (e == E_CANCELLED) return "E_CANCELLED";
    if (e == E_COMPLETED) return "E_COMPLETED";
    if (e == E_INVALID_ARGUMENT) return "E_INVALID_ARGUMENT";
    assert(!"Unknown error");
    return "<Unknown>";
}

std::ostream& Scheduler::operator<<(
    std::ostream& o,
    Error e)
{
    return o << ErrorToStr(e);
}

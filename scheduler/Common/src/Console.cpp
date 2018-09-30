#include <Scheduler/Common/Console.h>

#include <iostream>
#include <mutex>

Scheduler::Console::Console() : Console(std::cout) { }

Scheduler::Console::Console(std::ostream& out) : m_out(out) { }

Scheduler::Console::~Console() { Emit(); }

void Scheduler::Console::Emit() const
{
    static std::mutex s_consoleMutex;
    std::lock_guard<std::mutex> lock(s_consoleMutex);
    std::cout << m_stream.str();
}

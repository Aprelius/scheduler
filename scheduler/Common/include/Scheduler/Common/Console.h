#pragma once

#include <sstream>

namespace Scheduler {

    class Console
    {
    public:
        Console();
        Console(std::ostream& out);
        ~Console();

        template<typename T>
        std::ostream& operator<<(const T& value)
        {
            return m_stream << value;
        }

        std::ostream& operator<<(const char* value)
        {
            return m_stream << value;
        }

    private:
        void Emit() const;
        std::ostringstream m_stream;
        std::ostream& m_out;
    };

}  // namespae Scheduler

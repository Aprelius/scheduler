#pragma once

#include <iosfwd>
#include <string>

namespace Scheduler {
namespace Lib {

    class UUID
    {
    public:
        enum
        {
            SIZE = 16,
            LENGTH = 37
        };

        static UUID FromString(const char* data);

        explicit UUID(bool initialize = false);
        UUID(UUID&&) = default;
        UUID(const UUID& id) = default;
        ~UUID();

        UUID& operator=(UUID&&) = default;
        UUID& operator=(const UUID& id) = default;

        const uint8_t* Data() const { return m_data; }
        uint8_t* Data() { return m_data; }

        bool Empty() const { return !m_initialized; }

        void Initialize();

        bool IsValid() const { return Size() > 0; }

        std::string ToString(bool format = true) const;

        size_t Size() const { return static_cast<size_t>(m_size); }

        bool operator==(const UUID& id) const;
        bool operator!=(const UUID& id) const
        {
            return !operator==(id);
        }

        bool operator<(const UUID& id) const;
        bool operator>(const UUID& id) const { return !operator<(id); }

    private:
        uint8_t m_data[SIZE] = { 0 };
        uint8_t m_size = 0;
        bool m_initialized = false;
    };

    std::ostream& operator<<(std::ostream& o, const UUID& id);

}  // namespace Lib
}  // namespace Scheduler

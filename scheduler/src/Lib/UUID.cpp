#include <Scheduler/Lib/UUID.h>

#include <Scheduler/Common/ASCII.h>
#include <Scheduler/Common/Random.h>

#include <iostream>
#include <limits>
#include <string.h>

Scheduler::Lib::UUID Scheduler::Lib::UUID::FromString(
    const char* data)
{
    size_t length = std::min<size_t>(
        strlen(data),
        std::numeric_limits<uint8_t>::max());

    if (length % 2 > 0) return UUID();
    size_t hexLength = std::min<size_t>(length / 2, SIZE);

    for (size_t i = 0; i < length; ++i)
    {
        if (!IsHex(data[i])) return UUID();
    }

    UUID uuid;
    uint8_t hi = 0, lo = 0;
    for (uint8_t i = 0, j = 0; i < hexLength; ++i)
    {
        hi = static_cast<uint8_t>(ToNibble(data[j++]));
        lo = static_cast<uint8_t>(ToNibble(data[j++]));
        uuid.m_data[i] = (hi << 4) | lo;
    }

    uuid.m_initialized = true;
    uuid.m_size = static_cast<uint8_t>(hexLength);
    return uuid;
}

Scheduler::Lib::UUID::UUID(bool initialize)
{
    if (initialize) Initialize();
}

Scheduler::Lib::UUID::~UUID() { }

size_t Scheduler::Lib::UUID::Hash() const
{
    uint64_t hi, lo;
    memcpy(&hi, m_data, 8);
    memcpy(&lo, m_data + 8, 8);
    return static_cast<size_t>(hi + lo);
}

void Scheduler::Lib::UUID::Initialize()
{
    static Generator<uint8_t> generator(0, 15);

    if (m_initialized) return;

    uint8_t hi = 0, lo = 0;
    for (uint8_t i = 0; i < SIZE; ++i)
    {
        hi = static_cast<uint8_t>(ToNibble(hexChars[generator.Generate()]));
        lo = static_cast<uint8_t>(ToNibble(hexChars[generator.Generate()]));
        m_data[i] = (hi << 4) | lo;
    }

    m_initialized = true;
    m_size = SIZE;
}

std::string Scheduler::Lib::UUID::ToString(bool format) const
{
    char buffer[UUID::LENGTH + 1] = { 0 };
    const uint8_t* b = static_cast<const uint8_t*>(m_data);
    const size_t length = (format) ? LENGTH : LENGTH - 4;

    for (size_t i = 0; i < length - 1;)
    {
        if (format && (i == 8 || i == 13 || i == 18 || i == 23))
            buffer[i++] = '-';
        buffer[i++] = hexChars[(*b >> 4) & 0xF];
        buffer[i++] = hexChars[*b++ & 0xF];
    }

    return buffer;
}

bool Scheduler::Lib::UUID::operator==(const UUID& id) const
{
    return memcmp(m_data, id.m_data, SIZE) == 0;
}

bool Scheduler::Lib::UUID::operator<(const UUID& id) const
{
    return memcmp(m_data, id.m_data, SIZE) < 0;
}

std::ostream& Scheduler::Lib::operator<<(
    std::ostream& o,
    const UUID& id)
{
    return o << id.ToString();
}


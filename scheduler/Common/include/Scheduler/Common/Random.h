#pragma once

#include <Scheduler/Common/Clock.h>
#include <random>

namespace Scheduler {

    template<typename T>
    class Generator
    {
    public:
        Generator(T min, T max)
            : m_min(min),
              m_max(max)
        {
            m_dist = std::uniform_int_distribution<T>(min, max);
            m_engine = std::default_random_engine();
            m_engine.seed(Clock::now().time_since_epoch().count());
        }

        ~Generator() { }

        T Generate() const { return m_dist(m_engine); }

    private:
        T m_min = 0;
        T m_max = 0;
        mutable std::default_random_engine m_engine;
        mutable std::uniform_int_distribution<T> m_dist;
    };

}  // namespace Scheduler

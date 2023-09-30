#include "common/Timer.h"

#include "platform/time.h"

namespace
{
    size_t now()
    {
        return platform::get_monotonic_time_counter();
    }

    float getDeltaTime(size_t startTime, size_t stopTime, size_t frequency)
    {
        return 1.0f * (stopTime - startTime) / frequency;
    }
}

vkc::Timer::Timer()
{
    m_countsPerSecond = platform::get_monotonic_time_frequency();
    start();
}

void vkc::Timer::start()
{
    m_startTime = now();
}

float vkc::Timer::loop()
{
    size_t loopTime = now();
    float dt = getDeltaTime(m_startTime, loopTime, m_countsPerSecond);
    m_startTime = loopTime;
    return dt;
}

float vkc::Timer::getTime() const
{
    size_t currentTime = now();
    return getDeltaTime(m_startTime, currentTime, m_countsPerSecond);
}

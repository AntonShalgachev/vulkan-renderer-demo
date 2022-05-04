#include "Timer.h"

namespace
{
    inline float getDeltaTime(vkr::Timer::TimePointT startTime, vkr::Timer::TimePointT stopTime)
    {
        return std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }
}

vkr::Timer::Timer()
{
    start();
}

void vkr::Timer::start()
{
    m_startTime = ClockT::now();
}

float vkr::Timer::loop()
{
    TimePointT loopTime = ClockT::now();
    float dt = getDeltaTime(m_startTime, loopTime);
    m_startTime = loopTime;
    return dt;
}

float vkr::Timer::getTime() const
{
    TimePointT currentTime = ClockT::now();
    return getDeltaTime(m_startTime, currentTime);
}

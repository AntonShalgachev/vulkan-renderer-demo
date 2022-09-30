#include "Timer.h"

namespace
{
    inline float getDeltaTime(vkc::Timer::TimePointT startTime, vkc::Timer::TimePointT stopTime)
    {
        return std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }
}

vkc::Timer::Timer()
{
    start();
}

void vkc::Timer::start()
{
    m_startTime = ClockT::now();
}

float vkc::Timer::loop()
{
    TimePointT loopTime = ClockT::now();
    float dt = getDeltaTime(m_startTime, loopTime);
    m_startTime = loopTime;
    return dt;
}

float vkc::Timer::getTime() const
{
    TimePointT currentTime = ClockT::now();
    return getDeltaTime(m_startTime, currentTime);
}

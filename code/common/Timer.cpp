#include "Timer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <assert.h>

namespace
{
    size_t now()
    {
        LARGE_INTEGER counter;

        if (!QueryPerformanceCounter(&counter))
            assert(false);

        return counter.QuadPart;
    }

    float getDeltaTime(size_t startTime, size_t stopTime, size_t frequency)
    {
        return 1.0f * (stopTime - startTime) / frequency;
    }
}

vkc::Timer::Timer()
{
    {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency))
            assert(false);

        m_countsPerSecond = frequency.QuadPart;
    }

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

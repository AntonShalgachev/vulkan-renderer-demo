#pragma once
#include <chrono>

namespace vkc
{
    class Timer
    {
    public:
        using ClockT = std::chrono::high_resolution_clock;
        using TimePointT = std::chrono::time_point<ClockT>;

    	Timer();

        void start();
        float loop();

        float getTime() const;

    private:
        TimePointT m_startTime;
    };
}

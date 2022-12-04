#pragma once

namespace vkc
{
    class Timer
    {
    public:
    	Timer();

        void start();
        float loop();

        float getTime() const;

    private:
        size_t m_countsPerSecond = 0;
        size_t m_startTime = 0;
    };
}

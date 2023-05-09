#include "platform/time.h"

#include "windows_common.h"

namespace
{
    size_t get_frequency()
    {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency))
            assert(false);

        assert(frequency.QuadPart > 0);
        return static_cast<size_t>(frequency.QuadPart);
    }

    static size_t timer_frequency = get_frequency();
}

size_t platform::get_monotonic_time_frequency()
{
    return timer_frequency;
}

size_t platform::get_monotonic_time_counter()
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter))
        assert(false);

    assert(counter.QuadPart > 0);
    return static_cast<size_t>(counter.QuadPart);
}

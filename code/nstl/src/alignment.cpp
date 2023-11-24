#include "nstl/alignment.h"

#include <assert.h>

bool nstl::is_power_of_2(uint64_t x)
{
    return (x > 0) && ((x & (x - 1)) == 0);
}

uint64_t nstl::align_up(uint64_t value, size_t alignment)
{
    if (alignment == 0)
        return value;

    assert(is_power_of_2(alignment));
    return (value + alignment - 1) / alignment * alignment;
}

bool nstl::is_aligned(uint64_t value, size_t alignment)
{
    assert(is_power_of_2(alignment));
    return (value & (alignment - 1)) == 0;
}

bool nstl::is_aligned(void* ptr, size_t alignment)
{
    assert(is_power_of_2(alignment));
    return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
}

#include "common/align.h"

#include <assert.h>

bool common::is_power_of_2(uint64_t x)
{
    return (x > 0) && ((x & (x - 1)) == 0);
}

uint64_t common::align_up(uint64_t value, size_t alignment)
{
    assert(alignment > 0);
    return (value + alignment - 1) / alignment * alignment;
}

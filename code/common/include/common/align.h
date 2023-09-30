#pragma once

#include <stdint.h>

// TODO move somewhere

namespace common
{
    bool is_power_of_2(uint64_t x);
    uint64_t align_up(uint64_t value, size_t alignment);
}

#pragma once

#include <stdint.h>

namespace nstl
{
    bool is_power_of_2(uint64_t x);
    uint64_t align_up(uint64_t value, size_t alignment);
    bool is_aligned(uint64_t value, size_t alignment);
    bool is_aligned(void* ptr, size_t alignment);
}

#pragma once

namespace platform
{
    inline uint64_t atomic_fetch_increment_relaxed(uint64_t volatile& dest)
    {
        return __atomic_fetch_add(&dest, 1, __ATOMIC_RELAXED);
    }
}

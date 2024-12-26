#pragma once

#include <atomic>

#pragma intrinsic (_InterlockedIncrement64)

namespace platform
{
    inline uint64_t atomic_fetch_increment_relaxed(uint64_t volatile& dest)
    {
        int64_t volatile& signed_dest = reinterpret_cast<int64_t volatile&>(dest);
        return _InterlockedIncrement64(&signed_dest) - 1; // _InterlockedIncrement64 returns incremented value
    }
}

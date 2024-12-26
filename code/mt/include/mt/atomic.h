#pragma once

#include "platform/atomic.h"

namespace mt
{
    inline uint64_t atomic_fetch_increment_relaxed(uint64_t volatile& dest)
    {
        return platform::atomic_fetch_increment_relaxed(dest);
    }
}

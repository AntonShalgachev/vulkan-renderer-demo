#pragma once

#include "nstl/optional.h"

#include <stdint.h>

namespace platform
{
    using thread_handle = uint64_t;
    using thread_func_t = void(*)(void*);

    nstl::optional<thread_handle> create_thread(thread_func_t func, void* arg);
}

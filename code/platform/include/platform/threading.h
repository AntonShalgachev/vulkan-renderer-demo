#pragma once

#include "nstl/aligned_storage.h"

#include <stdint.h>

namespace platform
{
    // Thread
    using thread_storage_t = nstl::aligned_storage_t<8, 8>;
    using thread_func_t = void(*)(void*);

    [[nodiscard]] bool create_thread(thread_storage_t& storage, thread_func_t func, void* arg);
    uint64_t thread_get_current_id();

    // Mutex
    using mutex_storage_t = nstl::aligned_storage_t<40, 8>;

    [[nodiscard]] bool mutex_create(mutex_storage_t& storage);
    void mutex_destroy(mutex_storage_t& storage);
    void mutex_lock(mutex_storage_t& storage);
    void mutex_unlock(mutex_storage_t& storage);
}

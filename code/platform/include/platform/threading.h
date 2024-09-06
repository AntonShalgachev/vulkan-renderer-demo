#pragma once

#include "nstl/aligned_storage.h"

#include <stdint.h>

namespace nstl
{
    class string_view;
}

namespace platform
{
    // Thread
    using thread_storage_t = nstl::aligned_storage_t<8, 8>;
    using thread_func_t = void(*)(void*);

    [[nodiscard]] bool thread_create_empty(thread_storage_t& storage);
    [[nodiscard]] bool thread_create(thread_storage_t& storage, thread_func_t func, void* arg, nstl::string_view name);
    void thread_swap(thread_storage_t& lhs, thread_storage_t& rhs);
    void thread_join(thread_storage_t& storage);
    void thread_destroy(thread_storage_t& storage);
    uint64_t thread_get_current_id();

    void sleep(uint64_t milliseconds); // TODO: move somewhere else?

    // Mutex
    using mutex_storage_t = nstl::aligned_storage_t<40, 8>;

    [[nodiscard]] bool mutex_create(mutex_storage_t& storage);
    void mutex_destroy(mutex_storage_t& storage);
    void mutex_lock(mutex_storage_t& storage);
    void mutex_unlock(mutex_storage_t& storage);
}

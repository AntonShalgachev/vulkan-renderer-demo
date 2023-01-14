#pragma once

#include "nstl/string_view.h"
#include "nstl/span.h"

namespace memory
{
    namespace tracking
    {
        void on_scope_enter(nstl::string_view name);
        void on_scope_exit();
        nstl::string_view get_current_scope_name();

        struct scope_guard
        {
            scope_guard(nstl::string_view name)
            {
                memory::tracking::on_scope_enter(name);
            }

            ~scope_guard()
            {
                memory::tracking::on_scope_exit();
            }
        };

        struct scope_stat
        {
            nstl::string_view name;
            size_t bytes = 0;
            size_t active_allocations = 0;
            size_t total_allocations = 0;
        };

        void track_allocation(void* ptr, size_t size);
        void track_deallocation(void* ptr);

        nstl::span<scope_stat const> get_scope_stats();
    }
}

#define MEMORY_TRACKING_SCOPE(name) memory::tracking::scope_guard memory_tracking_scope_guard{name}

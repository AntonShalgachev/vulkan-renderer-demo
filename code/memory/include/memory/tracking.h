#pragma once

#include "nstl/string_view.h"
#include "nstl/vector.h"

namespace memory
{
    namespace tracking
    {
        struct scope_id
        {
            size_t index = static_cast<size_t>(-1);
        };

        enum class scope_type
        {
            internal,
            external,
        };

        scope_id create_scope_id(nstl::string_view name, scope_type type = scope_type::internal);
        nstl::string_view get_scope_name(scope_id id);

        void on_scope_enter(scope_id id);
        void on_scope_exit();
        scope_id get_current_thread_scope_id();

        struct scope_guard
        {
            scope_guard(scope_id id)
            {
                memory::tracking::on_scope_enter(id);
            }

            ~scope_guard()
            {
                memory::tracking::on_scope_exit();
            }
        };

        struct scope_stat
        {
            memory::tracking::scope_id id;
            size_t active_bytes = 0;
            size_t total_bytes = 0;
            size_t active_allocations = 0;
            size_t total_allocations = 0;
        };

        void track_allocation(void* ptr, size_t size);
        void track_deallocation(void* ptr);

        nstl::vector<scope_stat> get_scope_stats_copy();
    }
}

// TODO move somewhere
#define MEMORY_CONCATENATE(x, y) x ## y
#define MEMORY_CONCATENATE2(x, y) MEMORY_CONCATENATE(x, y)

#define MEMORY_TRACKING_SCOPE(id) memory::tracking::scope_guard MEMORY_CONCATENATE2(memory_tracking_scope_guard, __LINE__){id}

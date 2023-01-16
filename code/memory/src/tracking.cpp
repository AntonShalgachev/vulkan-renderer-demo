#include "memory/tracking.h"

#include "memory/untracked_allocator.h"

#include "logging/logging.h"

#include "nstl/vector.h"
#include "nstl/unordered_map.h"

namespace
{
    struct scope_parameters
    {
        nstl::string_view name;
        memory::tracking::scope_type type = memory::tracking::scope_type::internal;
    };

    nstl::vector<scope_parameters>& get_scope_parameters_container()
    {
        static nstl::vector<scope_parameters> params{ memory::untracked_allocator{} };
        return params;
    }

    scope_parameters const& get_scope_parameters(memory::tracking::scope_id id)
    {
        return get_scope_parameters_container()[id.index];
    }

    // TODO use static_vector
    nstl::vector<memory::tracking::scope_id>& get_scope_stack()
    {
        static nstl::vector<memory::tracking::scope_id> names{ memory::untracked_allocator{} };
        return names;
    }

    struct allocation_metadata
    {
        size_t size = 0;
        memory::tracking::scope_id scope_id;
        // TODO callstack id, etc.
    };

    nstl::unordered_map<void*, allocation_metadata>& get_allocation_metadata()
    {
        static nstl::unordered_map<void*, allocation_metadata> metadata{ memory::untracked_allocator{} };
        return metadata;
    }

    class scope_stats
    {
    public:
        scope_stats() = default;
        ~scope_stats()
        {
            for (memory::tracking::scope_stat const& entry : entries)
            {
                scope_parameters const& params = get_scope_parameters(entry.id);
                if (params.type == memory::tracking::scope_type::external)
                    continue;

                assert(entry.bytes == 0);
                assert(entry.active_allocations == 0);
            }

            logging::info("No memory leaks detected");
        }

        nstl::span<memory::tracking::scope_stat const> get_entries()
        {
            return entries;
        }

        void track_allocation(memory::tracking::scope_id id, size_t bytes)
        {
            assert(bytes > 0);

            for (size_t index = entries.size(); index <= id.index; index++)
                entries.push_back(memory::tracking::scope_stat{ .id = memory::tracking::scope_id{index} });

            assert(id.index < entries.size());
            memory::tracking::scope_stat& entry = entries[id.index];

            entry.bytes += bytes;
            entry.active_allocations++;
            entry.total_allocations++;
        }

        void track_deallocation(memory::tracking::scope_id id, size_t bytes)
        {
            assert(bytes > 0);

            assert(id.index < entries.size());
            memory::tracking::scope_stat& entry = entries[id.index];

            assert(entry.bytes >= bytes);
            entry.bytes -= bytes;

            assert(entry.active_allocations > 0);
            entry.active_allocations--;
        }

    private:
        nstl::vector<memory::tracking::scope_stat> entries{ memory::untracked_allocator{} };
    };

    scope_stats& get_scope_stats()
    {
        static scope_stats stats;
        return stats;
    }
}

memory::tracking::scope_id memory::tracking::create_scope_id(nstl::string_view name, scope_type type)
{
    scope_id id = {
        .index = get_scope_parameters_container().size()
    };

    get_scope_parameters_container().push_back(scope_parameters{
        .name = name,
        .type = type,
    });

    return id;
}

nstl::string_view memory::tracking::get_scope_name(scope_id id)
{
    return get_scope_parameters(id).name;
}

void memory::tracking::on_scope_enter(scope_id id)
{
    get_scope_stack().push_back(id);
}

void memory::tracking::on_scope_exit()
{
    get_scope_stack().pop_back();
}

memory::tracking::scope_id memory::tracking::get_current_scope_id()
{
    static scope_id const root_scope_id = create_scope_id("");

    if (get_scope_stack().empty())
        return root_scope_id;

    return get_scope_stack().back();
}

void memory::tracking::track_allocation(void* ptr, size_t size)
{
    assert(ptr);
    assert(size > 0);
    assert(get_allocation_metadata().find(ptr) == get_allocation_metadata().end());

    scope_id scope_id = get_current_scope_id();
    get_allocation_metadata()[ptr] = allocation_metadata{ size, scope_id };

    ::get_scope_stats().track_allocation(scope_id, size);
}

void memory::tracking::track_deallocation(void* ptr)
{
    assert(ptr);
    assert(get_allocation_metadata().find(ptr) != get_allocation_metadata().end());

    allocation_metadata const& metadata = get_allocation_metadata()[ptr];

    assert(metadata.size > 0);

    ::get_scope_stats().track_deallocation(metadata.scope_id, metadata.size);

    get_allocation_metadata().erase(ptr);
}

nstl::span<memory::tracking::scope_stat const> memory::tracking::get_scope_stats()
{
    return ::get_scope_stats().get_entries();
}

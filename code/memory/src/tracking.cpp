#include "memory/tracking.h"

#include "memory/untracked_allocator.h"

#include "logging/logging.h"

#include "nstl/vector.h"
#include "nstl/unordered_map.h"

namespace
{
    // TODO use static_vector
    nstl::vector<nstl::string_view>& get_scope_stack()
    {
        static nstl::vector<nstl::string_view> names{ memory::untracked_allocator{} };
        return names;
    }

    struct allocation_metadata
    {
        size_t size = 0;
        nstl::string_view scope_name;
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
                assert(entry.bytes == 0);
                assert(entry.active_allocations == 0);
            }

            // TODO
            logging::info("No memory leaks detected");
        }

        nstl::span<memory::tracking::scope_stat const> get_entries()
        {
            return entries;
        }

        void track_allocation(nstl::string_view name, size_t bytes)
        {
            memory::tracking::scope_stat* entry = find_entry(name);

            if (!entry)
                entry = &entries.emplace_back(memory::tracking::scope_stat{ name });

            entry->bytes += bytes;
            entry->active_allocations++;
            entry->total_allocations++;
        }

        void track_deallocation(nstl::string_view name, size_t bytes)
        {
            memory::tracking::scope_stat* entry = find_entry(name);
            assert(entry);

            assert(entry->bytes >= bytes);
            entry->bytes -= bytes;
            entry->active_allocations--;
        }

    private:
        memory::tracking::scope_stat* find_entry(nstl::string_view name)
        {
            for (auto& entry : entries)
                if (entry.name == name)
                    return &entry;
            return nullptr;
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

void memory::tracking::on_scope_enter(nstl::string_view name)
{
    get_scope_stack().push_back(name);
}

void memory::tracking::on_scope_exit()
{
    get_scope_stack().pop_back();
}

nstl::string_view memory::tracking::get_current_scope_name()
{
    if (get_scope_stack().empty())
        return {};

    return get_scope_stack().back();
}

void memory::tracking::track_allocation(void* ptr, size_t size)
{
    nstl::string_view scope_name = get_current_scope_name();
    get_allocation_metadata()[ptr] = allocation_metadata{ size, scope_name };

    ::get_scope_stats().track_allocation(scope_name, size);
}

void memory::tracking::track_deallocation(void* ptr)
{
    allocation_metadata const& metadata = get_allocation_metadata()[ptr];

    ::get_scope_stats().track_deallocation(metadata.scope_name, metadata.size);

    get_allocation_metadata().erase(ptr);
}

nstl::span<memory::tracking::scope_stat const> memory::tracking::get_scope_stats()
{
    return ::get_scope_stats().get_entries();
}

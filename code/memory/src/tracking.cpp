#include "memory/tracking.h"

#include "memory/untracked_allocator.h"

#include "logging/logging.h"

#include "mt/mutex.h"
#include "mt/lock_guard.h"

#include "nstl/vector.h"
#include "nstl/unordered_map.h"

namespace
{
    struct scope_parameters
    {
        nstl::string_view name;
        memory::tracking::scope_type type = memory::tracking::scope_type::internal;
    };

    struct allocation_metadata
    {
        size_t size = 0;
        memory::tracking::scope_id scope_id;
        // TODO callstack id, etc.
    };

    class teardown_tracker
    {
    public:
        static void on_teardown_started()
        {
            mt::lock_guard lock{ g_mutex };
            g_is_tearing_down = true;
        }

        static bool is_tearing_down()
        {
            mt::lock_guard lock{ g_mutex };
            return g_is_tearing_down;
        }

    private:
        static inline mt::mutex g_mutex;
        static inline bool g_is_tearing_down = false;
    };

    class global_tracking_data
    {
    public:
        global_tracking_data() = default;
        ~global_tracking_data()
        {
            teardown_tracker::on_teardown_started();

            mt::lock_guard lock{ m_mutex };

            for (memory::tracking::scope_stat const& entry : m_scope_stats)
            {
                scope_parameters const& params = m_scope_params[entry.id.index];
                if (params.type == memory::tracking::scope_type::external)
                    continue;

                assert(entry.active_bytes == 0);
                assert(entry.active_allocations == 0);
            }
        }

        nstl::string_view get_scope_name(memory::tracking::scope_id id)
        {
            mt::lock_guard lock{ m_mutex };

            return m_scope_params[id.index].name;
        }

        memory::tracking::scope_id create_scope_id(nstl::string_view name, memory::tracking::scope_type type)
        {
            mt::lock_guard lock{ m_mutex };

            memory::tracking::scope_id id = {
                .index = m_scope_params.size()
            };

            m_scope_params.push_back(scope_parameters{
                .name = name,
                .type = type,
                });

            return id;
        }

        void add_allocation_metadata(void* ptr, size_t size, memory::tracking::scope_id scope_id)
        {
            mt::lock_guard lock{ m_mutex };

            assert(m_allocation_metadata.find(ptr) == m_allocation_metadata.end());
            m_allocation_metadata[ptr] = allocation_metadata{ size, scope_id };
        }

        allocation_metadata get_allocation_metadata(void* ptr)
        {
            mt::lock_guard lock{ m_mutex };

            assert(ptr);
            assert(m_allocation_metadata.find(ptr) != m_allocation_metadata.end());

            return m_allocation_metadata[ptr];
        }

        void remove_allocation_metadata(void* ptr)
        {
            mt::lock_guard lock{ m_mutex };

            assert(ptr);
            assert(m_allocation_metadata.find(ptr) != m_allocation_metadata.end());

            m_allocation_metadata.erase(ptr);
        }

        void track_allocation(memory::tracking::scope_id id, size_t bytes)
        {
            mt::lock_guard lock{ m_mutex };

            assert(bytes > 0);

            for (size_t index = m_scope_stats.size(); index <= id.index; index++)
                m_scope_stats.push_back(memory::tracking::scope_stat{ .id = memory::tracking::scope_id{index} });

            assert(id.index < m_scope_stats.size());
            memory::tracking::scope_stat& entry = m_scope_stats[id.index];

            entry.active_bytes += bytes;
            entry.total_bytes += bytes;
            entry.active_allocations++;
            entry.total_allocations++;
        }

        void track_deallocation(memory::tracking::scope_id id, size_t bytes)
        {
            mt::lock_guard lock{ m_mutex };

            assert(bytes > 0);

            assert(id.index < m_scope_stats.size());
            memory::tracking::scope_stat& entry = m_scope_stats[id.index];

            assert(entry.active_bytes >= bytes);
            entry.active_bytes -= bytes;

            assert(entry.active_allocations > 0);
            entry.active_allocations--;
        }

        nstl::vector<memory::tracking::scope_stat> get_scope_stats_copy()
        {
            mt::lock_guard lock{ m_mutex };

            return m_scope_stats;
        }

    private:
        mt::mutex m_mutex;
        nstl::vector<scope_parameters> m_scope_params{ memory::untracked_allocator{} };
        nstl::unordered_map<void*, allocation_metadata> m_allocation_metadata{ memory::untracked_allocator{} };
        nstl::vector<memory::tracking::scope_stat> m_scope_stats{ memory::untracked_allocator{} };
    };

    class thread_tracking_data
    {
    public:
        thread_tracking_data() = default;

        void push_scope(memory::tracking::scope_id id)
        {
            m_scope_stack.push_back(id);
        }

        void pop_scope()
        {
            m_scope_stack.pop_back();
        }

        memory::tracking::scope_id get_current_scope_id()
        {
            static memory::tracking::scope_id const unsorted_scope_id = memory::tracking::create_scope_id("Unsorted");

            if (m_scope_stack.empty())
                return unsorted_scope_id;

            return m_scope_stack.back();
        }

    private:
        nstl::vector<memory::tracking::scope_id> m_scope_stack{ memory::untracked_allocator{} }; // TODO use static_vector
    };

    thread_tracking_data& get_thread_data()
    {
        assert(!teardown_tracker::is_tearing_down());

        static thread_local thread_tracking_data data;
        return data;
    }

    global_tracking_data& get_global_data()
    {
        static global_tracking_data data;
        return data;
    }
}

memory::tracking::scope_id memory::tracking::create_scope_id(nstl::string_view name, scope_type type)
{
    return get_global_data().create_scope_id(name, type);
}

nstl::string_view memory::tracking::get_scope_name(scope_id id)
{
    return get_global_data().get_scope_name(id);
}

void memory::tracking::on_scope_enter(scope_id id)
{
    get_thread_data().push_scope(id);
}

void memory::tracking::on_scope_exit()
{
    get_thread_data().pop_scope();
}

memory::tracking::scope_id memory::tracking::get_current_thread_scope_id()
{
    return get_thread_data().get_current_scope_id();
}

void memory::tracking::track_allocation(void* ptr, size_t size)
{
    assert(ptr);
    assert(size > 0);

    scope_id scope_id = get_current_thread_scope_id();

    get_global_data().add_allocation_metadata(ptr, size, scope_id);
    get_global_data().track_allocation(scope_id, size);
}

void memory::tracking::track_deallocation(void* ptr)
{
    global_tracking_data& data = get_global_data();

    allocation_metadata metadata = data.get_allocation_metadata(ptr);

    assert(metadata.size > 0);

    data.track_deallocation(metadata.scope_id, metadata.size);
    data.remove_allocation_metadata(ptr);
}

nstl::vector<memory::tracking::scope_stat> memory::tracking::get_scope_stats_copy()
{
    return ::get_global_data().get_scope_stats_copy();
}

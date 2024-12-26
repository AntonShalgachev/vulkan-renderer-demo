#include "mt/thread_id.h"

#include "mt/atomic.h"

namespace
{
    thread_local bool has_thread_id = false;
    thread_local uint64_t thread_id;

    uint64_t volatile next_thread_id;
}

uint64_t mt::get_thread_id()
{
    if (!has_thread_id)
        thread_id = mt::atomic_fetch_increment_relaxed(next_thread_id);

    return thread_id;
}

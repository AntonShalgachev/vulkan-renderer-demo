#include "memory/memory.h"

#include "platform/memory.h"

#include "memory/tracking.h"

#include "nstl/alignment.h"

void* memory::allocate(size_t size, size_t alignment)
{
    if (size == 0)
        return nullptr;

    void* ptr = platform::allocate(size, alignment);
    assert(ptr);
    assert(nstl::is_aligned(ptr, alignment));

    memory::tracking::track_allocation(ptr, size);

    return ptr;
}

void* memory::reallocate(void* ptr, size_t size, size_t alignment)
{
    // TODO implement track_reallocation?
    if (ptr)
        memory::tracking::track_deallocation(ptr);

    void* new_ptr = platform::reallocate(ptr, size, alignment);
    assert(nstl::is_aligned(new_ptr, alignment));

    // TODO implement track_reallocation?
    if (new_ptr && size > 0)
        memory::tracking::track_allocation(new_ptr, size);

    return new_ptr;
}

void memory::deallocate(void* ptr)
{
    if (ptr)
        memory::tracking::track_deallocation(ptr);

    return platform::deallocate(ptr);
}

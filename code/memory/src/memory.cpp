#include "memory/memory.h"

#include "platform/memory.h"

#include "memory/tracking.h"

void* memory::allocate(size_t size)
{
    assert(size > 0);

    void* ptr = platform::allocate(size);
    assert(ptr);

    memory::tracking::track_allocation(ptr, size);

    return ptr;
}

void* memory::reallocate(void* ptr, size_t size)
{
    void* newPtr = platform::reallocate(ptr, size);

    // TODO implement track_reallocation?
    if (ptr)
        memory::tracking::track_deallocation(ptr);
    if (newPtr && size > 0)
        memory::tracking::track_allocation(newPtr, size);

    return newPtr;
}

void memory::deallocate(void* ptr)
{
    if (ptr)
        memory::tracking::track_deallocation(ptr);

    return platform::deallocate(ptr);
}

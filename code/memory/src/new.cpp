#include <stdlib.h>

#include "platform/memory.h"

#include "memory/tracking.h"

void* operator new(size_t size)
{
    assert(size > 0);

    void* ptr = platform::allocate(size);
    assert(ptr);

    memory::tracking::track_allocation(ptr, size);

    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (ptr)
        memory::tracking::track_deallocation(ptr);

    return platform::deallocate(ptr);
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* ptr) noexcept
{
    return ::operator delete(ptr);
}

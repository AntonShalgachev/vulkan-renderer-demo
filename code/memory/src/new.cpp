#include <stdlib.h>

#include "memory/memory.h"

void* operator new(size_t size)
{
    return memory::allocate(size);
}

void operator delete(void* ptr) noexcept
{
    return memory::deallocate(ptr);
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* ptr) noexcept
{
    return ::operator delete(ptr);
}

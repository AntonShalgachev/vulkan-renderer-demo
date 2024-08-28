#include <stdlib.h>

#include "memory/memory.h"

#include <new>

void* operator new(size_t size)
{
    return memory::allocate(size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
}

void* operator new(size_t size, std::align_val_t align)
{
    return memory::allocate(size, static_cast<size_t>(align));
}

void operator delete(void* ptr) noexcept
{
    return memory::deallocate(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept
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

#pragma once

#include <stddef.h>

namespace memory
{
    void* allocate(size_t size, size_t alignment);
    void* reallocate(void* ptr, size_t size, size_t alignment);
    void deallocate(void* ptr);
}

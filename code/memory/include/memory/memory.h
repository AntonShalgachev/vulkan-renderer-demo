#pragma once

#include <stddef.h>

namespace memory
{
    void* allocate(size_t size);
    void* reallocate(void* ptr, size_t size);
    void deallocate(void* ptr);
}

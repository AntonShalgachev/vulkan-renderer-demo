#pragma once

namespace platform
{
    void* allocate(size_t size, size_t alignment);
    void* reallocate(void* ptr, size_t size, size_t alignment);
    void deallocate(void* ptr);
}

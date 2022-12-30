#pragma once

namespace nstl
{
    struct malloc_allocator
    {
        void* allocate(size_t size);
        void deallocate(void* ptr);
    };
}

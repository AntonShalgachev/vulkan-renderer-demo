#pragma once

namespace nstl
{
    // TODO rename to something like operator_new_allocator or default_allocator
    struct malloc_allocator
    {
        void* allocate(size_t size, size_t alignment);
        void deallocate(void* ptr);

        bool operator==(malloc_allocator const&) const;
    };
}

#pragma once

namespace memory
{
    struct untracked_allocator
    {
        void* allocate(size_t size, size_t alignment);
        void deallocate(void* ptr);
        bool operator==(untracked_allocator const&) const;
    };
}

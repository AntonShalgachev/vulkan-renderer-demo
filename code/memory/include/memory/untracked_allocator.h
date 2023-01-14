#pragma once

namespace memory
{
    struct untracked_allocator
    {
        void* allocate(size_t size);
        void deallocate(void* ptr);
        bool operator==(untracked_allocator const&) const;
    };
}

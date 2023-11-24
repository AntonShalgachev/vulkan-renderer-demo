#include "memory/untracked_allocator.h"

#include "platform/memory.h"

void* memory::untracked_allocator::allocate(size_t size, size_t alignment)
{
    return platform::allocate(size, alignment);
}

void memory::untracked_allocator::deallocate(void* ptr)
{
    return platform::deallocate(ptr);
}

bool memory::untracked_allocator::operator==(untracked_allocator const&) const
{
    return true;
}

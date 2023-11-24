#include "nstl/malloc_allocator.h"

#include "memory/memory.h"

void* nstl::malloc_allocator::allocate(size_t size, size_t alignment)
{
    return memory::allocate(size, alignment);
}

void nstl::malloc_allocator::deallocate(void* ptr)
{
    return memory::deallocate(ptr);
}

bool nstl::malloc_allocator::operator==(malloc_allocator const&) const
{
    return true;
}

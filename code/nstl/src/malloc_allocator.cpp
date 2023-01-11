#include "nstl/malloc_allocator.h"

void* nstl::malloc_allocator::allocate(size_t size)
{
    return ::operator new(size);
}

void nstl::malloc_allocator::deallocate(void* ptr)
{
    return ::operator delete(ptr);
}

bool nstl::malloc_allocator::operator==(malloc_allocator const&) const
{
    return true;
}

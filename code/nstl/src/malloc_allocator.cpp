#include "nstl/malloc_allocator.h"

#include <stdlib.h>

void* nstl::malloc_allocator::allocate(size_t size)
{
    return malloc(size);
}

void nstl::malloc_allocator::deallocate(void* ptr)
{
    free(ptr);
}

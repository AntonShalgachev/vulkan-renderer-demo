#include "platform/memory.h"

#include "nstl/alignment.h"

#include <assert.h>
#include <malloc.h>

// TODO use WinAPI?

void* platform::allocate(size_t size, size_t alignment)
{
    assert(nstl::is_power_of_2(alignment));
    return _aligned_malloc(size, alignment);
}

void* platform::reallocate(void* ptr, size_t size, size_t alignment)
{
    assert(nstl::is_power_of_2(alignment));
    return _aligned_realloc(ptr, size, alignment);
}

void platform::deallocate(void* ptr)
{
    return _aligned_free(ptr);
}

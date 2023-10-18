#include "platform/memory.h"

#include <stdlib.h>

// TODO use WinAPI?

void* platform::allocate(size_t size)
{
    return malloc(size);
}

void* platform::reallocate(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void platform::deallocate(void* ptr)
{
    return free(ptr);
}

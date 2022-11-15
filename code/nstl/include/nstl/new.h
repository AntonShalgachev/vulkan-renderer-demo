#pragma once

#include <stddef.h>

namespace nstl
{
    struct NewTag
    {
    };
}

void* operator new(size_t, nstl::NewTag, void* p);
void operator delete(void*, nstl::NewTag, void*);

#pragma once

#include <stddef.h>

namespace nstl
{
    struct new_tag
    {
    };
}

void* operator new(size_t, nstl::new_tag, void* p);
void operator delete(void*, nstl::new_tag, void*);

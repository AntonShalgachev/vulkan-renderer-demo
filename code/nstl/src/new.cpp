#include "nstl/new.h"

void* operator new(size_t, nstl::new_tag, void* p)
{
    return p;
}

void operator delete(void*, nstl::new_tag, void*) {} // @NOCOVERAGE

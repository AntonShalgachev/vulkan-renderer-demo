#include "nstl/new.h"

void* operator new(size_t, nstl::NewTag, void* p)
{
    return p;
}

void operator delete(void*, nstl::NewTag, void*) {} // @NOCOVERAGE

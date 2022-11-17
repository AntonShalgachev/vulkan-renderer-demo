#pragma once

#include "stddef.h"

namespace nstl
{
    template<typename T>
    struct hash
    {
        static_assert(sizeof(T) < 0, "No specialization of Hash for type T");
        size_t operator()(T const& value);
    };
}

#pragma once

#include "nstl/type_traits.h"

#include <string.h>

namespace nstl
{
    template <class InputIt, class T>
    constexpr InputIt find(InputIt first, InputIt last, T const& value)
    {
        for (; first != last; ++first)
            if (*first == value)
                return first;

        return last;
    }

    template <class InputIt, class OutputIt>
    constexpr OutputIt copy(InputIt first, InputIt last, OutputIt dest)
    {
        using T = simple_decay_t<decltype(*first)>;

        if constexpr (is_trivial_v<T>)
        {
            size_t count = last - first;
            memmove(dest, first, count * sizeof(T));
            return dest + count;
        }
        else
        {
            for (; first != last; ++first, ++dest)
                *dest = *first;
            return dest;
        }
    }
}

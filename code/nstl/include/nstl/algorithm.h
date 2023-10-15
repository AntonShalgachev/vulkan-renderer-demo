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

    template <class InputIt, class Pred>
    constexpr InputIt find_if(InputIt first, InputIt last, Pred const& pred)
    {
        for (; first != last; ++first)
            if (pred(*first))
                return first;

        return last;
    }

    template <typename InputIt, typename T>
    InputIt remove(InputIt first, InputIt last, const T& value)
    {
        first = nstl::find(first, last, value);
        if (first != last)
            for (InputIt i = first; ++i != last; )
                if (!(*i == value))
                    *first++ = nstl::move(*i);
        return first;
    }

    template <typename InputIt, typename Pred>
    InputIt remove_if(InputIt first, InputIt last, Pred const& p)
    {
        first = nstl::find_if(first, last, p);
        if (first != last)
            for (InputIt i = first; ++i != last; )
                if (!p(*i))
                    *first++ = nstl::move(*i);
        return first;
    }

    template <class InputIt, class OutputIt>
    constexpr OutputIt copy(InputIt first, InputIt last, OutputIt dest)
    {
        NSTL_ASSERT(first <= last);

        using T = simple_decay_t<decltype(*first)>;

        if constexpr (is_trivial_v<T>)
        {
            size_t count = static_cast<size_t>(last - first);
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

    template<class T>
    T const& min(T const& a, T const& b)
    {
        return (a < b) ? a : b;
    }

    template<class T>
    T const& max(T const& a, T const& b)
    {
        return (a > b) ? a : b;
    }

    template<typename T>
    T const& clamp(T const& value, T const& from, T const& to)
    {
        if (value < from)
            return from;
        if (value > to)
            return to;
        return value;
    };
}

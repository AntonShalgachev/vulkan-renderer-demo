#pragma once

#include "TypeTraits.h"

namespace nstl
{
    template<typename T>
    constexpr RemoveReferenceT<T>&& move(T&& arg) noexcept
    {
        return static_cast<RemoveReferenceT<T>&&>(arg);
    }

    template<typename T>
    constexpr T&& forward(RemoveReferenceT<T>& arg) noexcept
    {
        return static_cast<T&&>(arg);
    }

    template<typename T>
    constexpr T&& forward(RemoveReferenceT<T>&& arg) noexcept
    {
        return static_cast<T&&>(arg);
    }

    template<typename T>
    void exchange(T& lhs, T& rhs) noexcept
    {
        T temp = nstl::move(lhs);
        lhs = nstl::move(rhs);
        rhs = nstl::move(temp);
    }
}

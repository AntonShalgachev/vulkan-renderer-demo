#pragma once

#include "type_traits.h"

namespace nstl
{
    template<typename T>
    constexpr remove_reference_t<T>&& move(T&& arg) noexcept
    {
        return static_cast<remove_reference_t<T>&&>(arg);
    }

    template<typename T>
    constexpr T&& forward(remove_reference_t<T>& arg) noexcept
    {
        return static_cast<T&&>(arg);
    }

    template<typename T>
    constexpr T&& forward(remove_reference_t<T>&& arg) noexcept
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

    template<typename T, typename U = T&&> U declval_impl();
    template<typename T> auto declval() noexcept -> decltype(declval_impl<T>());
}

#pragma once

#include "nstl/type_traits.h"

namespace nstl
{
    template<typename T> struct type_tag {};

    namespace detail
    {
        template<nstl::enumeration E> constexpr bool nstl_is_flags_enum(type_tag<E>) { return false; }
    }

    template<nstl::enumeration E>
    constexpr bool is_flags_enum()
    {
        using namespace detail;
        return nstl_is_flags_enum(type_tag<E>{});
    }

    template<typename E> concept flags_enumeration = is_flags_enum<E>();
}

#define NSTL_FLAGS_ENUM(E) constexpr bool nstl_is_flags_enum(nstl::type_tag<E>) { return true; }
#define NSTL_NESTED_FLAGS_ENUM(E) friend constexpr bool nstl_is_flags_enum(nstl::type_tag<E>) { return true; }

template<nstl::flags_enumeration E>
inline E operator~(E v)
{
    using T = nstl::underlying_type_t<E>;
    return static_cast<E>(~static_cast<T>(v));
}

template<nstl::flags_enumeration E>
inline E operator|(E lhs, E rhs)
{
    using T = nstl::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template<nstl::flags_enumeration E>
inline E& operator|=(E& lhs, E rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<nstl::flags_enumeration E>
inline E operator&(E lhs, E rhs)
{
    using T = nstl::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template<nstl::flags_enumeration E>
inline E& operator&=(E& lhs, E rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

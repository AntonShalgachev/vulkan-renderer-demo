#pragma once

#include "nstl/type_traits.h"

// TODO extend operators

template<nstl::enumeration E>
inline E operator|(E lhs, E rhs)
{
    using T = nstl::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template<nstl::enumeration E>
inline E& operator|=(E& lhs, E rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<nstl::enumeration E>
inline auto operator&(E lhs, E rhs)
{
    using T = nstl::underlying_type_t<E>;
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}

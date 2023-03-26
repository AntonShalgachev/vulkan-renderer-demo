#pragma once

#include <stddef.h>

namespace nstl
{
    template<typename T, T... Is>
    struct integer_sequence {};

    template<size_t... Is>
    using index_sequence = integer_sequence<size_t, Is...>;

#if defined(__clang__) || defined(_MSC_VER)
    template<size_t size>
    using make_index_sequence = __make_integer_seq<integer_sequence, size_t, size>;
#elif defined(__GNUC__) && __has_builtin(__make_integer_seq)
    template<size_t size>
    using make_index_sequence = __make_integer_seq<integer_sequence, size_t, size>;
#elif defined(__GNUC__)
    template<size_t size>
    using make_index_sequence = integer_sequence<size_t, __integer_pack(size)...>;
#endif
}

#pragma once

#include "picofmt/config.h"

namespace picofmt
{
    struct writer;

    template<typename T, typename = void>
    struct formatter
    {
        // TODO use is_formattable_v instead of this assert
        static_assert(sizeof(T) < 0, "formatter isn't implemented for type T");

        bool parse(string_view specifier, writer& ctx);
        bool format(T const& value, writer& ctx);
    };

    template<typename T>
    constexpr bool is_formattable_v = false; // TODO implement
}

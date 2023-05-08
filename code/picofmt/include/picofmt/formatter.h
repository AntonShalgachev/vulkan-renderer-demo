#pragma once

#include "picofmt/config.h"

namespace picofmt
{
    class writer;
    class args_list;
    class context;

#if defined(__clang__)
    template<typename T1, typename T2> concept same_as = __is_same(T1, T2);
#else
    template<typename T1, typename T2> inline constexpr bool is_same_v = false;
    template<typename T> inline constexpr bool is_same_v<T, T> = true;
    template<typename T1, typename T2> concept same_as = is_same_v<T1, T2>;
#endif

    // The library expects the formatter to look like this:
    // struct formatter<T>
    // {
    //     bool parse(string_view specifier, context& ctx);
    //     bool format(T const& value, context& ctx) const;
    // };

    template<typename T>
    struct formatter;

    template<typename T>
    concept formattable = requires(formatter<T>& fmt, formatter<T> const& const_fmt, T const& value, string_view specifier, context& ctx)
    {
        { fmt.parse(specifier, ctx) } -> same_as<bool>;
        { const_fmt.format(value, ctx) } -> same_as<bool>;
    };
}

#pragma once

#include "common/tiny_ctti.h"

#include "nstl/vector.h"
#include "nstl/string.h"
#include "nstl/string_view.h"

#define PICOFMT_CUSTOM_STRING_VIEW nstl::string_view
#include "picofmt/picofmt.h"

#include <ctype.h>

// TODO move formatters out of here?

template<>
struct picofmt::formatter<nstl::string> : public picofmt::formatter<nstl::string_view> {};

namespace common::detail
{
    struct string_case_formatter : picofmt::formatter<nstl::string_view>
    {
        bool parse(nstl::string_view specifier, picofmt::context& ctx);
        bool format(nstl::string_view const& value, picofmt::context& ctx) const;
        bool upper_case = false;
    };
}

template<tiny_ctti::described_enum E>
struct picofmt::formatter<E> : public common::detail::string_case_formatter
{
    bool format(E const& value, context& ctx) const
    {
        return common::detail::string_case_formatter::format(tiny_ctti::enum_name(value), ctx);
    }
};

template<picofmt::formattable T>
struct picofmt::formatter<nstl::span<T const>> : public picofmt::formatter<T>
{
    bool format(nstl::span<T const> const& values, context& ctx) const
    {
        ctx.write("[");

        bool write_delimiter = false;
        for (T const& value : values)
        {
            if (write_delimiter)
                ctx.write(", ");

            if (!picofmt::formatter<T>::format(value, ctx))
                return false;

            write_delimiter = true;
        }

        ctx.write("]");

        return true;
    }
};

template<picofmt::formattable T>
struct picofmt::formatter<nstl::span<T>> : public picofmt::formatter<nstl::span<T const>> {};

template<picofmt::formattable T>
struct picofmt::formatter<nstl::vector<T>> : public picofmt::formatter<nstl::span<T const>> {};

namespace common
{
    nstl::string vformat(nstl::string_view format, picofmt::args_list const& args);

    template<picofmt::formattable... Ts>
    nstl::string format(nstl::string_view format, Ts const&... args)
    {
        return vformat(format, picofmt::args_list{ args... });
    }
}

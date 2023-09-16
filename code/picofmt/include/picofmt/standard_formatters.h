#pragma once

#include "picofmt/config.h"
#include "picofmt/formatter.h"
#include "picofmt/generic_format_spec.h"
#include "picofmt/generic_format_spec_parser.h"

#include "picofmt/detail/standard_formatters_impl.h"

namespace picofmt
{
    namespace detail
    {
        struct standard_formatter_base
        {
            bool parse(string_view specifier, context& ctx)
            {
                return parse_generic_format_spec(specifier, format_spec, ctx);
            }

            generic_format_spec format_spec;
        };
    }

#define PICOFMT_CREATE_FORMATTER(T) template<> struct formatter<T> : detail::standard_formatter_base { bool format(T const& value, context& ctx) const { return format_value(value, format_spec, ctx); } };

    PICOFMT_CREATE_FORMATTER(void*);
    
    PICOFMT_CREATE_FORMATTER(bool);

    PICOFMT_CREATE_FORMATTER(char);
    PICOFMT_CREATE_FORMATTER(signed char);
    PICOFMT_CREATE_FORMATTER(unsigned char);

    PICOFMT_CREATE_FORMATTER(short);
    PICOFMT_CREATE_FORMATTER(unsigned short);
    PICOFMT_CREATE_FORMATTER(int);
    PICOFMT_CREATE_FORMATTER(unsigned int);
    PICOFMT_CREATE_FORMATTER(long);
    PICOFMT_CREATE_FORMATTER(unsigned long);
    PICOFMT_CREATE_FORMATTER(long long);
    PICOFMT_CREATE_FORMATTER(unsigned long long);

    PICOFMT_CREATE_FORMATTER(float);
    PICOFMT_CREATE_FORMATTER(double);
    PICOFMT_CREATE_FORMATTER(long double);

    PICOFMT_CREATE_FORMATTER(detail::simple_string_view);

#undef PICOFMT_CREATE_FORMATTER

    template<>
    struct formatter<string_view> : formatter<detail::simple_string_view>
    {
        bool format(string_view const& value, context& ctx) const
        {
            return formatter<detail::simple_string_view>::format({ value.data(), value.length() }, ctx);
        }
    };

    template<>
    struct formatter<char const*> : formatter<detail::simple_string_view> {};

    template<>
    struct formatter<char*> : formatter<detail::simple_string_view> {};

    template<size_t N>
    struct formatter<char[N]> : formatter<detail::simple_string_view>
    {
        bool format(char const (&value)[N], context& ctx) const
        {
            return formatter<detail::simple_string_view>::format({ value, N }, ctx);
        }
    };
}

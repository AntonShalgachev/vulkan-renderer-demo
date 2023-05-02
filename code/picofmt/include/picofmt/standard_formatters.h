#pragma once

#include "picofmt/config.h"
#include "picofmt/formatter.h"
#include "picofmt/generic_format_spec.h"
#include "picofmt/generic_format_spec_parser.h"

#include "picofmt/detail/writer_adapter.h"
#include "picofmt/detail/standard_formatters_impl.h"

namespace picofmt
{
#define PICOFMT_CREATE_FORMATTER(T)                                                                                              \
    template<> struct formatter<T>                                                                                               \
    {                                                                                                                            \
        bool parse(string_view specifier, writer& ctx) { return parse_generic_format_spec(specifier, format_spec, ctx); }        \
        bool format(T const& value, writer& ctx) const { return detail::format_value(value, detail::writer_adapter{ ctx }); }    \
        generic_format_spec format_spec;                                                                                         \
    }

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

    PICOFMT_CREATE_FORMATTER(char*);
    PICOFMT_CREATE_FORMATTER(char const*);

#undef PICOFMT_CREATE_FORMATTER

    template<size_t N>
    struct formatter<char[N]> : formatter<char const*> {}; // TODO probably a bad idea since char[N] might not be null-terminated
}

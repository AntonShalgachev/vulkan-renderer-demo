#pragma once

// TODO rename file?

namespace picofmt
{
    struct generic_format_spec;

    namespace detail
    {
        struct context_base;
        struct simple_string_view;

#define PICOFMT_DECLARE_FORMATTER(T) bool format_value(T const& value, generic_format_spec const& format_spec, context_base const& ctx);

        PICOFMT_DECLARE_FORMATTER(bool);
        PICOFMT_DECLARE_FORMATTER(char);

        PICOFMT_DECLARE_FORMATTER(signed char);
        PICOFMT_DECLARE_FORMATTER(unsigned char);
        PICOFMT_DECLARE_FORMATTER(short);
        PICOFMT_DECLARE_FORMATTER(unsigned short);
        PICOFMT_DECLARE_FORMATTER(int);
        PICOFMT_DECLARE_FORMATTER(unsigned int);
        PICOFMT_DECLARE_FORMATTER(long);
        PICOFMT_DECLARE_FORMATTER(unsigned long);
        PICOFMT_DECLARE_FORMATTER(long long);
        PICOFMT_DECLARE_FORMATTER(unsigned long long);

        PICOFMT_DECLARE_FORMATTER(float);
        PICOFMT_DECLARE_FORMATTER(double);
        PICOFMT_DECLARE_FORMATTER(long double);

        PICOFMT_DECLARE_FORMATTER(simple_string_view);

#undef PICOFMT_DECLARE_FORMATTER
    }
}

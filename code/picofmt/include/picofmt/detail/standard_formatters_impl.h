#pragma once

// TODO rename file?

namespace picofmt::detail
{
    struct simple_writer;

#define PICOFMT_DECLARE_FORMATTER(T) bool format_value(T const& value, simple_writer const& ctx);

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

    PICOFMT_DECLARE_FORMATTER(char*);
    PICOFMT_DECLARE_FORMATTER(char const*);

#undef PICOFMT_DECLARE_FORMATTER
}

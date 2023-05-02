#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/detail/standard_formatters_impl.h"
#include "picofmt/detail/simple_writer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

namespace
{
#if defined(__clang__) || defined(__GNUC__)
#define PICOFMT_PRINTF_LIKE(string_index, first_to_check) __attribute__((__format__(__printf__, string_index, first_to_check)))
#else
#define PICOFMT_PRINTF_LIKE(string_index, first_to_check)
#endif

    bool vformat_to_ctx(picofmt::detail::simple_writer const& ctx, char const* fmt, va_list args)
    {
        // TODO don't use vsnprintf
        // TODO don't allocate temporary memory

        size_t size = static_cast<size_t>(vsnprintf(nullptr, 0, fmt, args));

        char* buffer = new char[size + 1];

        int result = vsnprintf(buffer, size + 1, fmt, args);
        assert(buffer[size] == '\0');
        assert(result == size);

        ctx.write(picofmt::detail::simple_string_view{ buffer, size });

        delete[] buffer;

        return true;
    }

    PICOFMT_PRINTF_LIKE(2, 3) bool format_to_ctx(picofmt::detail::simple_writer const& ctx, char const* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        bool result = vformat_to_ctx(ctx, fmt, args);
        va_end(args);

        return result;
    }
}

#define PICOFMT_DEFINE_FORMATTER(T, fmt) bool picofmt::detail::format_value(T const& value, simple_writer const& ctx) { return format_to_ctx(ctx, fmt, value); }

PICOFMT_DEFINE_FORMATTER(char, "%c");
PICOFMT_DEFINE_FORMATTER(signed char, "%hhd");
PICOFMT_DEFINE_FORMATTER(unsigned char, "%hhu");
PICOFMT_DEFINE_FORMATTER(short, "%hd");
PICOFMT_DEFINE_FORMATTER(unsigned short, "%hu");
PICOFMT_DEFINE_FORMATTER(int, "%d");
PICOFMT_DEFINE_FORMATTER(unsigned int, "%u");
PICOFMT_DEFINE_FORMATTER(long, "%ld");
PICOFMT_DEFINE_FORMATTER(unsigned long, "%lu");
PICOFMT_DEFINE_FORMATTER(long long, "%lld");
PICOFMT_DEFINE_FORMATTER(unsigned long long, "%llu");

PICOFMT_DEFINE_FORMATTER(float, "%f");
PICOFMT_DEFINE_FORMATTER(double, "%f");
PICOFMT_DEFINE_FORMATTER(long double, "%Lf");

PICOFMT_DEFINE_FORMATTER(char*, "%s");
PICOFMT_DEFINE_FORMATTER(char const*, "%s");

#undef PICOFMT_DEFINE_FORMATTER

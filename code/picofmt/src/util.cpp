#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/detail/util.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/context_base.h"

#include <stdlib.h>
#include <assert.h>

bool picofmt::detail::parse_index(simple_string_view str, size_t& value)
{
    size_t parsed_chars;
    if (!parse_index(str, parsed_chars, value))
        return false;

    return parsed_chars == str.length;
}

bool picofmt::detail::parse_index(simple_string_view str, size_t& parsed_chars, size_t& value)
{
    if (str.empty())
    {
        parsed_chars = 0;
        return false;
    }

    // TODO don't allocate and don't use strtoXXX

    char* null_terminated_str = new char[str.length + 1];

    memcpy(null_terminated_str, str.data, str.length);
    null_terminated_str[str.length] = '\0';

    errno = 0;

    char* parsed_end = nullptr;
    unsigned long long raw_value = strtoull(null_terminated_str, &parsed_end, 10);

    assert(parsed_end >= null_terminated_str);
    parsed_chars = static_cast<size_t>(parsed_end - null_terminated_str);

    delete[] null_terminated_str;

    if (parsed_chars == 0)
        return false;

    if (errno == ERANGE)
        return false;

    if (raw_value > SIZE_MAX)
        return false;

    value = static_cast<size_t>(raw_value);

    return true;
}

bool picofmt::detail::parse_nonnegative_int(simple_string_view str, size_t& parsed_chars, int& value)
{
    size_t raw_value = 0;
    if (!parse_index(str, parsed_chars, raw_value))
        return false;

    if (raw_value > INT_MAX)
        return false;

    value = static_cast<int>(raw_value);
    return true;
}

bool picofmt::detail::try_extract_replacement_field(simple_string_view str, size_t& closing_pos, context_base const& ctx)
{
    assert(!str.empty());
    assert(str[0] == '{');

    size_t current_depth = 1;
    for (size_t i = 1; i < str.length; i++)
    {
        if (str[i] == '{')
            current_depth++;

        if (str[i] != '}')
            continue;

        // TODO handle }}

        assert(current_depth > 0);
        current_depth--;

        if (current_depth == 0)
        {
            closing_pos = i;
            return true;
        }
    }

    if (current_depth > 0)
    {
        ctx.report_error("Unmatched '{' in format spec");
        return false;
    }

    assert(false);
    ctx.report_error("Internal error");
    return false;
}

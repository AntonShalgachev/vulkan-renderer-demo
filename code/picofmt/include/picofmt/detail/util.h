#pragma once

// TODO merge with core_impl.h

namespace picofmt::detail
{
    struct simple_string_view;
    class context_base;

    bool parse_index(simple_string_view str, size_t& value);
    bool parse_index(simple_string_view str, size_t& parsed_chars, size_t& value);

    bool parse_nonnegative_int(simple_string_view str, size_t& parsed_chars, int& value);

    bool try_extract_replacement_field(simple_string_view str, size_t& closing_pos, context_base const& ctx);
}

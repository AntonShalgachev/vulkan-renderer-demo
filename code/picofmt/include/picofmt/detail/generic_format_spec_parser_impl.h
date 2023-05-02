#pragma once

namespace picofmt
{
    struct generic_format_spec;

    namespace detail
    {
        struct simple_string_view;
        bool parse_generic_format_spec(simple_string_view specifier, generic_format_spec& format_spec);
    }
}

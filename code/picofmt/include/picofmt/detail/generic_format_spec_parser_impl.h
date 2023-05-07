#pragma once

namespace picofmt
{
    struct generic_format_spec;
    class args_list;

    namespace detail
    {
        struct simple_string_view;
        class context_base;
        bool parse_generic_format_spec(simple_string_view specifier, generic_format_spec& format_spec, context_base& ctx);
    }
}

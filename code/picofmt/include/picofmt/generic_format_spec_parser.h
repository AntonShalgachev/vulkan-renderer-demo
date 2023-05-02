#pragma once

#include "picofmt/config.h"
#include "picofmt/writer.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/generic_format_spec_parser_impl.h"

namespace picofmt
{
    inline bool parse_generic_format_spec(string_view specifier, generic_format_spec& format_spec, writer& ctx)
    {
        if (!detail::parse_generic_format_spec({ specifier.data(), specifier.length() }, format_spec))
        {
            ctx.report_error("Invalid type specifier");
            return false;
        }

        return true;
    }
}

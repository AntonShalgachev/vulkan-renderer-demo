#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/generic_format_spec.h"

#include "picofmt/detail/generic_format_spec_parser_impl.h"
#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/context_base.h"
#include "picofmt/detail/util.h"
#include "picofmt/detail/core_impl.h"

#include <assert.h>

namespace
{
    bool is_ascii_letter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool try_parse_presentation_type(char c, picofmt::presentation_type& type)
    {
        switch (c)
        {
        case 'd': type = picofmt::presentation_type::dec; return true;
        case 'o': type = picofmt::presentation_type::oct; return true;
        case 'x': type = picofmt::presentation_type::hex_lower; return true;
        case 'X': type = picofmt::presentation_type::hex_upper; return true;
        case 'b': type = picofmt::presentation_type::bin_lower; return true;
        case 'B': type = picofmt::presentation_type::bin_upper; return true;
        case 'a': type = picofmt::presentation_type::hexfloat_lower; return true;
        case 'A': type = picofmt::presentation_type::hexfloat_upper; return true;
        case 'e': type = picofmt::presentation_type::exp_lower; return true;
        case 'E': type = picofmt::presentation_type::exp_upper; return true;
        case 'f': type = picofmt::presentation_type::fixed_lower; return true;
        case 'F': type = picofmt::presentation_type::fixed_upper; return true;
        case 'g': type = picofmt::presentation_type::general_lower; return true;
        case 'G': type = picofmt::presentation_type::general_upper; return true;
        case 'c': type = picofmt::presentation_type::chr; return true;
        case 's': type = picofmt::presentation_type::string; return true;
        }

        return false;
    }

    picofmt::detail::simple_string_view parse_align(picofmt::detail::simple_string_view str, picofmt::generic_format_spec& format_spec)
    {
        assert(!str.empty());

        auto align = picofmt::align::none;

        size_t pos = 1;
        if (str.length <= 1)
            pos = 0;

        picofmt::detail::simple_string_view rest = str;

        // TODO rewrite this loop
        for (;;)
        {
            switch (str[pos])
            {
            case '<': align = picofmt::align::left; break;
            case '>': align = picofmt::align::right; break;
            case '^': align = picofmt::align::center; break;
            case '=': align = picofmt::align::numeric; break;
            }

            if (align != picofmt::align::none)
            {
                if (pos != 0)
                {
                    format_spec.fill = str[pos - 1];

                    rest = str.substr(pos + 1);
                }
                else
                {
                    rest = str.substr(1);
                }

                format_spec.align = align;
                break;
            }
            else if (pos == 0)
            {
                break;
            }

            pos = 0;
        }

        return rest;
    }

    bool try_parse_sign(char c, picofmt::sign& sign)
    {
        switch (c)
        {
        case '+': sign = picofmt::sign::plus; return true;
        case '-': sign = picofmt::sign::minus; return true;
        case ' ': sign = picofmt::sign::space; return true;
        }

        return false;
    }

    bool parse_dynamic_param(picofmt::detail::simple_string_view str, int& value, picofmt::detail::simple_string_view& rest, picofmt::detail::context_base& ctx)
    {
        assert(!str.empty());

        size_t parsed_chars = 0;
        if (picofmt::detail::parse_nonnegative_int(str, parsed_chars, value))
        {
            rest = str.substr(parsed_chars);
            return true;
        }

        // No value -> either the string doesn't contain any digits or the integer is out of range

        if (parsed_chars > 0)
        {
            ctx.report_error("Width out of range");
            return false;
        }

        if (str[0] == '{')
        {
            size_t closing_pos = 0;
            if (!picofmt::detail::try_extract_replacement_field(str, closing_pos, ctx))
                return false;

            assert(closing_pos > 0);
            assert(str[closing_pos] == '}');

            picofmt::detail::simple_string_view arg_id_str = str.substr(1, closing_pos - 1);

            size_t arg_id = 0;
            if (arg_id_str.empty())
            {
                arg_id = ctx.get_next_arg_id();
            }
            else
            {
                if (!picofmt::detail::parse_index(arg_id_str, arg_id))
                {
                    ctx.report_error("Invalid argument id"); // TODO include `arg_id_str`
                    return false;
                }
            }

            if (!ctx.args.try_get_int(arg_id, value))
            {
                // TODO report proper error?
                ctx.report_error("Argument not found or not an integer");
                return false;
            }

            ctx.consume_arg(arg_id);

            rest = str.substr(closing_pos + 1);
            return true;
        }

        rest = str;
        return true;
    }
}

bool picofmt::detail::parse_generic_format_spec(simple_string_view specifier, generic_format_spec& format_spec, context_base& ctx)
{
    if (specifier.empty())
        return true;

    if (specifier.length == 1 && is_ascii_letter(specifier[0]))
    {
        if (!try_parse_presentation_type(specifier[0], format_spec.type))
        {
            ctx.report_error("Unknown presentation type");
            return false;
        }

        return true;
    }

    specifier = parse_align(specifier, format_spec);

    if (specifier.empty())
        return true;

    if (try_parse_sign(specifier[0], format_spec.sign))
        specifier = specifier.substr(1);

    if (specifier.empty())
        return true;

    if (specifier[0] == '#')
    {
        format_spec.alternative_representation = true;
        specifier = specifier.substr(1);
    }

    if (specifier.empty())
        return true;

    if (specifier[0] == '0')
    {
        format_spec.fill = '0';
        if (format_spec.align == align::none)
            format_spec.align = align::numeric;

        specifier = specifier.substr(1);
    }

    if (specifier.empty())
        return true;

    if (!parse_dynamic_param(specifier, format_spec.width, specifier, ctx))
        return false;

    if (specifier.empty())
        return true;

    if (specifier[0] == '.')
    {
        specifier = specifier.substr(1);

        if (!parse_dynamic_param(specifier, format_spec.precision, specifier, ctx))
            return false;

        if (format_spec.precision < 0)
        {
            ctx.report_error("Mising precision specifier");
        }
    }

    if (specifier.empty())
        return true;

    if (!try_parse_presentation_type(specifier[0], format_spec.type))
    {
        ctx.report_error("Invalid format specifier");
        return false;
    }

    specifier = specifier.substr(1);

    if (!specifier.empty())
    {
        ctx.report_error("Invalid format specifier");
        return false;
    }

    return true;
}

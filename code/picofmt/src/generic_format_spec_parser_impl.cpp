#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/generic_format_spec.h"

#include "picofmt/detail/generic_format_spec_parser_impl.h"
#include "picofmt/detail/simple_string_view.h"

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
        assert(str.size() >= 1);

        auto align = picofmt::align::none;

        size_t pos = 1;
        if (str.size() <= 1)
            pos = 0;

        picofmt::detail::simple_string_view rest;

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

    //     picofmt::string_view parse_width(const Char* begin, const Char* end, Handler&& handler)
    //     {
    //         using detail::auto_id;
    //         struct width_adapter {
    //             Handler& handler;
    // 
    //             FMT_CONSTEXPR void operator()() { handler.on_dynamic_width(auto_id()); }
    //             FMT_CONSTEXPR void operator()(int id) { handler.on_dynamic_width(id); }
    //             FMT_CONSTEXPR void operator()(basic_picofmt::string_view<Char> id) {
    //                 handler.on_dynamic_width(id);
    //             }
    //             FMT_CONSTEXPR void on_error(const char* message) {
    //                 if (message) handler.on_error(message);
    //             }
    //         };
    // 
    //         FMT_ASSERT(begin != end, "");
    //         if ('0' <= *begin && *begin <= '9')
    //         {
    //             int width = parse_nonnegative_int(begin, end, -1);
    //             if (width != -1)
    //                 handler.on_width(width);
    //             else
    //                 handler.on_error("number is too big");
    //         }
    //         else if (*begin == '{')
    //         {
    //             ++begin;
    //             if (begin != end) begin = parse_arg_id(begin, end, width_adapter{ handler });
    //             if (begin == end || *begin != '}')
    //                 return handler.on_error("invalid format string"), begin;
    //             ++begin;
    //         }
    //         return begin;
    //     }

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
}

bool picofmt::detail::parse_generic_format_spec(simple_string_view specifier, generic_format_spec& format_spec)
{
    if (specifier.empty())
        return true;

    if (specifier.size() == 1 && is_ascii_letter(specifier[0]) && specifier[0] != 'L')
    {
        if (!try_parse_presentation_type(specifier[0], format_spec.type))
        {
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

    //         begin = parse_width(begin, end, handler);
    //         if (begin == end) return begin;
    // 
    //         // Parse precision.
    //         if (*begin == '.')
    //         {
    //             begin = parse_precision(begin, end, handler);
    //             if (begin == end) return begin;
    //         }
    // 
    //         if (*begin == 'L')
    //         {
    //             handler.on_localized();
    //             ++begin;
    //         }
    // 
    //         // Parse type.
//         if (begin != end && *begin != '}')
    //         {
    //             presentation_type type = parse_presentation_type(*begin++);
    //             if (type == presentation_type::none)
    //                 handler.on_error("invalid type specifier");
    //             handler.on_type(type);
    //         }
    //         return begin;

    return true;
}

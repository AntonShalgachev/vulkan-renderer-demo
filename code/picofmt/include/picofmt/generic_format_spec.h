#pragma once

#include <assert.h>

namespace picofmt
{
    enum class presentation_type
    {
        none,
        dec,             // 'd'
        oct,             // 'o'
        hex_lower,       // 'x'
        hex_upper,       // 'X'
        bin_lower,       // 'b'
        bin_upper,       // 'B'
        hexfloat_lower,  // 'a'
        hexfloat_upper,  // 'A'
        exp_lower,       // 'e'
        exp_upper,       // 'E'
        fixed_lower,     // 'f'
        fixed_upper,     // 'F'
        general_lower,   // 'g'
        general_upper,   // 'G'
        chr,             // 'c'
        string,          // 's'
        pointer,         // 'p'
    };

    enum class align
    {
        none,
        left,
        right,
        center,
        numeric,
    };

    enum class sign
    {
        minus,
        plus,
        space,
    };

    struct generic_format_spec
    {
        presentation_type type = presentation_type::none;

        align align = align::none;
        char fill = ' ';

        sign sign = sign::minus;

        bool alternative_representation = false;

        // TODO make it size_t?
        int width = -1;
        int precision = -1;
    };

    inline bool is_integer_type(presentation_type type)
    {
        switch (type)
        {
        case presentation_type::none: assert(false); return false;

        case presentation_type::dec:
        case presentation_type::oct:
        case presentation_type::hex_lower:
        case presentation_type::hex_upper:
        case presentation_type::bin_lower:
        case presentation_type::bin_upper:
            return true;

        case presentation_type::hexfloat_lower:
        case presentation_type::hexfloat_upper:
        case presentation_type::exp_lower:
        case presentation_type::exp_upper:
        case presentation_type::fixed_lower:
        case presentation_type::fixed_upper:
        case presentation_type::general_lower:
        case presentation_type::general_upper:
            return false;

        case presentation_type::chr: return false;
        case presentation_type::string: return false;
        case presentation_type::pointer: return false;
        }

        assert(false);
        return false;
    }
}

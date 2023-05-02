#pragma once

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
        none,
        minus,
        plus,
        space,
    };

    struct generic_format_spec
    {
        presentation_type type = presentation_type::none;

        align align = align::none;
        char fill = ' ';

        sign sign = sign::none;

        bool alternative_representation = false;
    };
}

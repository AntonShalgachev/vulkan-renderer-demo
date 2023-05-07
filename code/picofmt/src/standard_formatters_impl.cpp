#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/detail/standard_formatters_impl.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/context_base.h"

#include "picofmt/generic_format_spec.h"

#include <stdio.h>
#include <assert.h>
#include <float.h>

namespace
{
    template<typename T> struct make_unsigned;
    template<> struct make_unsigned<signed char> { using type = unsigned char; };
    template<> struct make_unsigned<unsigned char> { using type = unsigned char; };
    template<> struct make_unsigned<short> { using type = unsigned short; };
    template<> struct make_unsigned<unsigned short> { using type = unsigned short; };
    template<> struct make_unsigned<int> { using type = unsigned int; };
    template<> struct make_unsigned<unsigned int> { using type = unsigned int; };
    template<> struct make_unsigned<long> { using type = unsigned long; };
    template<> struct make_unsigned<unsigned long> { using type = unsigned long; };
    template<> struct make_unsigned<long long> { using type = unsigned long long; };
    template<> struct make_unsigned<unsigned long long> { using type = unsigned long long; };

    template<typename T> using make_unsigned_t = typename make_unsigned<T>::type;

    struct integer_formats_data
    {
        char const* dec_fmt = nullptr;
        char const* oct_fmt = nullptr;
        char const* hex_lower_fmt = nullptr;
        char const* hex_upper_fmt = nullptr;

        constexpr bool operator==(integer_formats_data const&) const = default;
    };

    template<typename T> constexpr integer_formats_data integer_formats_v;
    template<> constexpr integer_formats_data integer_formats_v<unsigned char> = { "%hhu", "%hho", "%hhx", "%hhX" };
    template<> constexpr integer_formats_data integer_formats_v<unsigned short> = { "%hu", "%ho", "%hx", "%hX" };
    template<> constexpr integer_formats_data integer_formats_v<unsigned int> = { "%u", "%o", "%x", "%X" };
    template<> constexpr integer_formats_data integer_formats_v<unsigned long> = { "%lu", "%lo", "%lx", "%lX" };
    template<> constexpr integer_formats_data integer_formats_v<unsigned long long> = { "%llu", "%llo", "%llx", "%llX" };

    struct float_formats_data
    {
        char const* fixed_upper_fmt = nullptr;
        char const* fixed_lower_fmt = nullptr;
        char const* exp_upper_fmt = nullptr;
        char const* exp_lower_fmt = nullptr;
        char const* general_upper_fmt = nullptr;
        char const* general_lower_fmt = nullptr;
        char const* hexfloat_upper_fmt = nullptr;
        char const* hexfloat_lower_fmt = nullptr;

        constexpr bool operator==(float_formats_data const&) const = default;
    };

    template<typename T> constexpr float_formats_data float_formats_v;
    template<> constexpr float_formats_data float_formats_v<float> = { "%.*f", "%.*F", "%.*e", "%.*E", "%.*g", "%.*G", "%.*a", "%.*A" };
    template<> constexpr float_formats_data float_formats_v<double> = { "%.*f", "%.*F", "%.*e", "%.*E", "%.*g", "%.*G", "%.*a", "%.*A" };
    template<> constexpr float_formats_data float_formats_v<long double> = { "%.*Lf", "%.*LF", "%.*Le", "%.*LE", "%.*Lg", "%.*LG", "%.*La", "%.*LA" };

#if !defined(LDBL_DECIMAL_DIG)
#if !defined(DECIMAL_DIG)
#error Both LDBL_DECIMAL_DIG and DECIMAL_DIG aren't defined
#endif
#define LDBL_DECIMAL_DIG DECIMAL_DIG
#endif

    template<typename T> constexpr int float_decimal_digits_v = -1;
    template<> constexpr int float_decimal_digits_v<float> = FLT_DECIMAL_DIG;
    template<> constexpr int float_decimal_digits_v<double> = DBL_DECIMAL_DIG;
    template<> constexpr int float_decimal_digits_v<long double> = LDBL_DECIMAL_DIG;

    struct presentation_params
    {
        char const* fmt = nullptr;
        picofmt::detail::simple_string_view radix_prefix;
    };

    bool calculate_presentation_params(picofmt::presentation_type type, integer_formats_data const& formats, presentation_params& params)
    {
        switch (type)
        {
        case picofmt::presentation_type::none: params = { formats.dec_fmt, {} }; return true;

        case picofmt::presentation_type::dec: params = { formats.dec_fmt, {} }; return true;
        case picofmt::presentation_type::oct: params = { formats.oct_fmt, "0" }; return true;
        case picofmt::presentation_type::hex_lower: params = { formats.hex_lower_fmt, "0x" }; return true;
        case picofmt::presentation_type::hex_upper: params = { formats.hex_upper_fmt, "0X" }; return true;
        case picofmt::presentation_type::bin_lower: return false; // TODO
        case picofmt::presentation_type::bin_upper: return false; // TODO

        case picofmt::presentation_type::hexfloat_lower: return false;
        case picofmt::presentation_type::hexfloat_upper: return false;
        case picofmt::presentation_type::exp_lower: return false;
        case picofmt::presentation_type::exp_upper: return false;
        case picofmt::presentation_type::fixed_upper: return false;
        case picofmt::presentation_type::fixed_lower: return false;
        case picofmt::presentation_type::general_upper: return false;
        case picofmt::presentation_type::general_lower: return false;

        case picofmt::presentation_type::chr: return false;
        case picofmt::presentation_type::string: return false;
        }

        return false;
    }

    bool calculate_presentation_params(picofmt::presentation_type type, float_formats_data const& formats, presentation_params& params)
    {
        switch (type)
        {
        case picofmt::presentation_type::none: params = { formats.general_lower_fmt, {} }; return true;
        case picofmt::presentation_type::dec: return false;
        case picofmt::presentation_type::oct: return false;
        case picofmt::presentation_type::hex_lower: return false;
        case picofmt::presentation_type::hex_upper: return false;
        case picofmt::presentation_type::bin_lower: return false;
        case picofmt::presentation_type::bin_upper: return false;

        case picofmt::presentation_type::hexfloat_lower: params = { formats.hexfloat_lower_fmt, {} }; return true;
        case picofmt::presentation_type::hexfloat_upper: params = { formats.hexfloat_upper_fmt, {} }; return true;
        case picofmt::presentation_type::exp_lower: params = { formats.exp_lower_fmt, {} }; return true;
        case picofmt::presentation_type::exp_upper: params = { formats.exp_upper_fmt, {} }; return true;
        case picofmt::presentation_type::fixed_upper: params = { formats.fixed_upper_fmt, {} }; return true;
        case picofmt::presentation_type::fixed_lower: params = { formats.fixed_lower_fmt, {} }; return true;
        case picofmt::presentation_type::general_upper: params = { formats.general_upper_fmt, {} }; return true;
        case picofmt::presentation_type::general_lower: params = { formats.general_lower_fmt, {} }; return true;

        case picofmt::presentation_type::chr: return false;
        case picofmt::presentation_type::string: return false;
        }

        return false;
    }

    bool write_value_string(picofmt::generic_format_spec const& format_spec, picofmt::detail::simple_string_view value_str, picofmt::detail::simple_string_view prefix_str, picofmt::detail::context_base const& ctx)
    {
        size_t prefix_value_length = prefix_str.length + value_str.length;

        size_t left_fill_length = 0;
        size_t middle_fill_length = 0;
        size_t right_fill_length = 0;

        if (format_spec.width > 0 && format_spec.width > prefix_value_length)
        {
            size_t fill_width = static_cast<size_t>(format_spec.width) - prefix_value_length;

            picofmt::align align = format_spec.align;
            if (align == picofmt::align::none)
                align = picofmt::align::right;

            switch (align)
            {
            case picofmt::align::none:
                assert(false);
                break;
            case picofmt::align::left:
                left_fill_length = 0;
                middle_fill_length = 0;
                right_fill_length = fill_width;
                break;
            case picofmt::align::center:
                left_fill_length = fill_width / 2;
                middle_fill_length = 0;
                right_fill_length = fill_width - left_fill_length;
                break;
            case picofmt::align::right:
                left_fill_length = fill_width;
                middle_fill_length = 0;
                right_fill_length = 0;
                break;
            case picofmt::align::numeric:
                left_fill_length = 0;
                middle_fill_length = fill_width;
                right_fill_length = 0;
                break;
            }

            assert(left_fill_length + middle_fill_length + right_fill_length + prefix_value_length == format_spec.width);
        }

        if (left_fill_length > 0)
            if (!ctx.write(format_spec.fill, left_fill_length))
                return false;
        if (!ctx.write(prefix_str))
            return false;
        if (middle_fill_length > 0)
            if (!ctx.write(format_spec.fill, middle_fill_length))
                return false;
        if (!ctx.write(value_str))
            return false;
        if (right_fill_length > 0)
            if (!ctx.write(format_spec.fill, right_fill_length))
                return false;

        return true;
    }

    picofmt::detail::simple_string_view create_integer_prefix(char* prefix_buffer, size_t prefix_buffer_size, picofmt::generic_format_spec const& format_spec, bool is_negative, picofmt::detail::simple_string_view radix_prefix)
    {
        size_t prefix_length = 0;
        auto append_prefix = [&prefix_length, &prefix_buffer, &prefix_buffer_size](picofmt::detail::simple_string_view str)
        {
            size_t write_pos = prefix_length;
            prefix_length += str.length;
            assert(prefix_length <= prefix_buffer_size);

            memcpy(prefix_buffer + write_pos, str.data, str.length);
        };

        switch (format_spec.sign)
        {
        case picofmt::sign::plus:
            append_prefix(is_negative ? "-" : "+");
            break;
        case picofmt::sign::minus:
            if (is_negative)
                append_prefix("-");
            break;
        case picofmt::sign::space:
            append_prefix(is_negative ? "-" : " ");
            break;
        }

        if (format_spec.alternative_representation)
            append_prefix(radix_prefix);

        return { prefix_buffer, prefix_length };
    }

    template<typename T>
    bool format_integer(T const& value, picofmt::generic_format_spec const& format_spec, picofmt::detail::context_base const& ctx)
    {
        using UT = make_unsigned_t<T>;

        static_assert(integer_formats_v<UT> != integer_formats_data{}, "format strings aren't defined for type UT");

        bool is_negative = value < T{ 0 };

        presentation_params params;
        if (!calculate_presentation_params(format_spec.type, integer_formats_v<UT>, params))
        {
            ctx.report_error("Presentation type not supported for this type");
            return false;
        }

        constexpr size_t value_buffer_size = sizeof(T) * 8 + 1; // binary representation is the most extreme case, including NULL
        char value_buffer[value_buffer_size];

        int written_chars = snprintf(value_buffer, value_buffer_size, params.fmt, static_cast<UT>(is_negative ? -value : value));
        if (written_chars < 0)
        {
            ctx.report_error("Unknown error"); // TODO test how it can happen
            return false;
        }

        size_t value_length = static_cast<size_t>(written_chars);
        assert(value_length < value_buffer_size);
        picofmt::detail::simple_string_view value_str = { value_buffer, value_length };

        constexpr size_t prefix_buffer_size = 3;
        char prefix_buffer[prefix_buffer_size];
        picofmt::detail::simple_string_view prefix_str = create_integer_prefix(prefix_buffer, prefix_buffer_size, format_spec, is_negative, params.radix_prefix);

        return write_value_string(format_spec, value_str, prefix_str, ctx);
    }

    template<typename T>
    bool format_float(T const& value, picofmt::generic_format_spec const& format_spec, picofmt::detail::context_base const& ctx)
    {
        static_assert(float_formats_v<T> != float_formats_data{}, "format strings aren't defined for type T");

        bool is_negative = value < T{ 0 };

        presentation_params params;
        if (!calculate_presentation_params(format_spec.type, float_formats_v<T>, params))
        {
            ctx.report_error("Presentation type not supported for this type");
            return false;
        }

        constexpr size_t value_buffer_size = sizeof(T) * 8 + 1; // TODO calculate it propery
        char value_buffer[value_buffer_size];

        int precision = format_spec.precision;
        if (format_spec.type == picofmt::presentation_type::none && precision < 0)
            precision = float_decimal_digits_v<T>;

        int written_chars = snprintf(value_buffer, value_buffer_size, params.fmt, precision, is_negative ? -value : value);
        if (written_chars < 0)
        {
            ctx.report_error("Unknown error"); // TODO test how it can happen
            return false;
        }

        size_t value_length = static_cast<size_t>(written_chars);
        assert(value_length < value_buffer_size);
        picofmt::detail::simple_string_view value_str = { value_buffer, value_length };

        constexpr size_t prefix_buffer_size = 3;
        char prefix_buffer[prefix_buffer_size];
        picofmt::detail::simple_string_view prefix_str = create_integer_prefix(prefix_buffer, prefix_buffer_size, format_spec, is_negative, params.radix_prefix);

        return write_value_string(format_spec, value_str, prefix_str, ctx);
    }
}

bool picofmt::detail::format_value(bool const& value, generic_format_spec const& format_spec, context_base const& ctx)
{
    if (format_spec.type == presentation_type::none || format_spec.type == presentation_type::string)
        return format_value(simple_string_view{ value ? "true" : "false" }, format_spec, ctx);

    if (is_integer_type(format_spec.type))
        return format_integer(static_cast<unsigned short>(value), format_spec, ctx);
    
    ctx.report_error("Presentation type not supported for this type");
    return false;
}

bool picofmt::detail::format_value(char const& value, generic_format_spec const& format_spec, context_base const& ctx)
{
    if (format_spec.type == presentation_type::none || format_spec.type == presentation_type::string)
        return format_value(simple_string_view{ &value, 1 }, format_spec, ctx);

    if (is_integer_type(format_spec.type))
        return format_integer(static_cast<int>(value), format_spec, ctx);

    ctx.report_error("Presentation type not supported for this type");
    return false;
}

#define PICOFMT_DEFINE_INTEGER_FORMATTER(T) bool picofmt::detail::format_value(T const& value, generic_format_spec const& format_spec, context_base const& ctx) { return format_integer(value, format_spec, ctx); }
#define PICOFMT_DEFINE_FLOAT_FORMATTER(T) bool picofmt::detail::format_value(T const& value, generic_format_spec const& format_spec, context_base const& ctx) { return format_float(value, format_spec, ctx); }

PICOFMT_DEFINE_INTEGER_FORMATTER(signed char);
PICOFMT_DEFINE_INTEGER_FORMATTER(unsigned char);
PICOFMT_DEFINE_INTEGER_FORMATTER(short);
PICOFMT_DEFINE_INTEGER_FORMATTER(unsigned short);
PICOFMT_DEFINE_INTEGER_FORMATTER(int);
PICOFMT_DEFINE_INTEGER_FORMATTER(unsigned int);
PICOFMT_DEFINE_INTEGER_FORMATTER(long);
PICOFMT_DEFINE_INTEGER_FORMATTER(unsigned long);
PICOFMT_DEFINE_INTEGER_FORMATTER(long long);
PICOFMT_DEFINE_INTEGER_FORMATTER(unsigned long long);

PICOFMT_DEFINE_FLOAT_FORMATTER(float);
PICOFMT_DEFINE_FLOAT_FORMATTER(double);
PICOFMT_DEFINE_FLOAT_FORMATTER(long double);

bool picofmt::detail::format_value(simple_string_view const& str, generic_format_spec const& format_spec, context_base const& ctx)
{
    if (format_spec.type != presentation_type::none && format_spec.type != presentation_type::string)
    {
        ctx.report_error("Presentation type not supported for this type");
        return false;
    }

    simple_string_view value_str = str;
    if (format_spec.precision > 0)
        value_str = value_str.substr(0, static_cast<size_t>(format_spec.precision));

    if (!write_value_string(format_spec, value_str, {}, ctx))
        return false;

    return true;
}

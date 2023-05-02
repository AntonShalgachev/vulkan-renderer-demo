#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/detail/core_impl.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/simple_writer.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

bool picofmt::detail::parse_int(simple_string_view str, int& value)
{
    char* null_terminated_str = new char[str.size() + 1];

    memcpy(null_terminated_str, str.data(), str.size());
    null_terminated_str[str.size()] = '\0';

    errno = 0;

    char* parsed_end = nullptr;
    long long_value = strtol(null_terminated_str, &parsed_end, 10);

    delete[] null_terminated_str;

    if (parsed_end == null_terminated_str)
        return false;

    if (parsed_end != null_terminated_str + str.size())
        return false;

    if (errno == ERANGE)
        return false;

    if (long_value > INT_MAX || long_value < INT_MIN)
        return false;

    value = static_cast<int>(long_value);
    return true;
}

bool picofmt::detail::vformat_to(simple_writer const& ctx, simple_string_view fmt, args_list const& args)
{
    size_t next_char_pos = 0;

    enum class arg_mode
    {
        unknown,
        manual,
        automatic,
    };

    arg_mode mode = arg_mode::unknown;
    size_t next_arg_index = 0;

    while (next_char_pos < fmt.size())
    {
        size_t replacement_field_begin_pos = fmt.find('{', next_char_pos);
        if (replacement_field_begin_pos == simple_string_view::npos)
        {
            break;
        }

        // Write everything that was before the replacement field
        simple_string_view str = fmt.substr(next_char_pos, replacement_field_begin_pos - next_char_pos);
        if (!str.empty())
            ctx.write(str);

        size_t replacement_field_end_pos = fmt.find('}', replacement_field_begin_pos);
        if (replacement_field_end_pos == simple_string_view::npos)
        {
            ctx.report_error("Unmatched '{' in format spec");
            return false;
        }

        assert(replacement_field_end_pos > replacement_field_begin_pos);
        simple_string_view replacement_field = fmt.substr(replacement_field_begin_pos + 1, replacement_field_end_pos - replacement_field_begin_pos - 1);

        assert(replacement_field_end_pos + 1 > next_char_pos); // to ensure that the loop will terminate
        next_char_pos = replacement_field_end_pos + 1;

        simple_string_view arg_id_str = replacement_field;
        simple_string_view format_spec_str{};

        size_t delimiter_pos = replacement_field.find(':');
        if (delimiter_pos != simple_string_view::npos)
        {
            arg_id_str = replacement_field.substr(0, delimiter_pos);
            format_spec_str = replacement_field.substr(delimiter_pos + 1);
        }

        arg_mode this_arg_mode = arg_id_str.empty() ? arg_mode::automatic : arg_mode::manual;

        if (mode == arg_mode::manual && this_arg_mode == arg_mode::automatic)
        {
            ctx.report_error("Cannot switch from manual field specification to automatic field numbering");
            return false;
        }
        else if (mode == arg_mode::automatic && this_arg_mode == arg_mode::manual)
        {
            ctx.report_error("Cannot switch from automatic field numbering to manual field specification");
            return false;
        }

        mode = this_arg_mode;

        assert(mode != arg_mode::unknown);

        size_t arg_index = next_arg_index;
        if (!arg_id_str.empty())
        {
            int value = 0;
            if (!detail::parse_int(arg_id_str, value))
            {
                ctx.report_error("Invalid argument id"); // TODO include `arg_id_str`
                return false;
            }

            if (value < 0)
            {
                ctx.report_error("Argument id can't be negative"); // TODO include `arg_id_str`
                return false;
            }

            arg_index = static_cast<size_t>(value);
        }

        if (!args[arg_index].parse(format_spec_str, ctx))
        {
            ctx.report_error("Invalid format specifier"); // TODO include `format_spec_str`
            return false;
        }

        if (!args[arg_index].format(ctx))
        {
            ctx.report_error("Unable to format argument"); // TODO include `arg_index`
            return false;
        }

        next_arg_index++;
    }

    simple_string_view str = fmt.substr(next_char_pos);
    if (!str.empty())
        ctx.write(str);

    return true;
}

picofmt::args_list::~args_list()
{
    for (size_t i = 0; i < m_size; i++)
        delete m_args[i];

    delete[] m_args;
}

picofmt::detail::any_arg& picofmt::args_list::operator[](size_t index) const
{
    assert(index < m_size);
    assert(m_args[index]);
    return *m_args[index];
}

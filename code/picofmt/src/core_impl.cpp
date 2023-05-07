#define PICOFMT_INTERNAL_IMPLEMENTATION

#include "picofmt/detail/core_impl.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/context_base.h"
#include "picofmt/detail/util.h"

#include <string.h>
#include <assert.h>

bool picofmt::detail:: vformat_to(simple_string_view fmt, context_base& ctx)
{
    simple_string_view rest = fmt;

    while (!rest.empty())
    {
        size_t replacement_field_begin_pos = rest.find('{');
        if (replacement_field_begin_pos == simple_string_view::npos)
            break;

        // TODO handle '{{'?

        // Write everything that was before the replacement field
        simple_string_view str = rest.substr(0, replacement_field_begin_pos);
        if (!str.empty())
            if (!ctx.write(str))
                return false;

        rest = rest.substr(replacement_field_begin_pos);

        assert(rest[0] == '{');

        size_t replacement_field_end_pos = 0;
        if (!try_extract_replacement_field(rest, replacement_field_end_pos, ctx))
            return false;

        assert(replacement_field_end_pos > 0);

        // Strip {} from the replacement field
        simple_string_view replacement_field = rest.substr(1, replacement_field_end_pos - 1);

        rest = rest.substr(replacement_field_end_pos + 1);

        simple_string_view arg_id_str = replacement_field;
        simple_string_view format_spec_str{};

        size_t delimiter_pos = replacement_field.find(':');
        if (delimiter_pos != simple_string_view::npos)
        {
            arg_id_str = replacement_field.substr(0, delimiter_pos);
            format_spec_str = replacement_field.substr(delimiter_pos + 1);
        }

        size_t arg_index = ctx.get_next_arg_id();

        if (!arg_id_str.empty())
        {
            if (!detail::parse_index(arg_id_str, arg_index))
            {
                ctx.report_error("Invalid argument id"); // TODO include `arg_id_str`
                return false;
            }
        }

        ctx.consume_arg(arg_index);

        if (!ctx.args[arg_index].parse(format_spec_str, ctx.get_user_context()))
        {
            ctx.report_error("Invalid format specifier"); // TODO include `format_spec_str`
            return false;
        }

        if (!ctx.args[arg_index].format(ctx.get_user_context()))
        {
            ctx.report_error("Unable to format argument"); // TODO include `arg_index`
            return false;
        }
    }

    if (!rest.empty())
        if (!ctx.write(rest))
            return false;

    return true;
}

picofmt::args_list::~args_list()
{
    for (size_t i = 0; i < m_size; i++)
        delete m_args[i];

    delete[] m_args;
}

bool picofmt::args_list::try_get_int(size_t index, int& value) const
{
    if (index >= m_size)
        return false;

    return m_args[index]->try_get_int(value);
}

picofmt::detail::any_arg& picofmt::args_list::operator[](size_t index) const
{
    assert(index < m_size);
    assert(m_args[index]);
    return *m_args[index];
}

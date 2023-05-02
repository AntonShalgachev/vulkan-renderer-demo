#pragma once

#include "picofmt/writer.h"
#include "picofmt/config.h"
#include "picofmt/formatter.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/core_impl.h"
#include "picofmt/detail/writer_adapter.h"

// TODO add constexpr validation

namespace picofmt
{
    namespace detail
    {
        template<typename T>
        struct any_arg_impl : any_arg
        {
            any_arg_impl(T const& value) : m_value(value) {}

            bool parse(simple_string_view specifier, simple_writer const& ctx) override
            {
                return m_formatter.parse(string_view{ specifier.data(), specifier.size() }, ctx.get_user_writer());
            }

            bool format(simple_writer const& ctx) const override
            {
                return m_formatter.format(m_value, ctx.get_user_writer());
            }

            T const& m_value;
            formatter<T> m_formatter;
        };
    }

    template<typename... Ts>
    args_list::args_list(Ts const& ... args)
        : m_size(sizeof...(args))
        , m_args(new detail::any_arg*[m_size])
    {
        size_t i = 0;
        ((m_args[i++] = new detail::any_arg_impl<Ts>(args)), ...);
    }

    //////////////////////////////////////////////////////////////////////////

    inline bool vformat_to(writer& ctx, string_view fmt, args_list const& args)
    {
        return detail::vformat_to(detail::writer_adapter{ ctx }, detail::simple_string_view{ fmt.data(), fmt.length() }, args);
    }

    template<typename... Ts>
    bool format_to(writer& ctx, string_view fmt, Ts const&... args)
    {
        return vformat_to(ctx, fmt, args_list{ args... });
    }
}

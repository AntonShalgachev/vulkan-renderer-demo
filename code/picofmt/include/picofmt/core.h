#pragma once

#include "picofmt/context.h"
#include "picofmt/config.h"
#include "picofmt/formatter.h"

#include "picofmt/detail/simple_string_view.h"
#include "picofmt/detail/core_impl.h"
#include "picofmt/detail/context_base.h"

// TODO add constexpr validation

namespace picofmt
{
    namespace detail
    {
        template<typename T>
        inline constexpr bool is_integer_v = false;

        template<> inline constexpr bool is_integer_v<signed char> = true;
        template<> inline constexpr bool is_integer_v<unsigned char> = true;
        template<> inline constexpr bool is_integer_v<short> = true;
        template<> inline constexpr bool is_integer_v<unsigned short> = true;
        template<> inline constexpr bool is_integer_v<int> = true;
        template<> inline constexpr bool is_integer_v<unsigned int> = true;
        template<> inline constexpr bool is_integer_v<long> = true;
        template<> inline constexpr bool is_integer_v<unsigned long> = true;
        template<> inline constexpr bool is_integer_v<long long> = true;
        template<> inline constexpr bool is_integer_v<unsigned long long> = true;

        inline string_view to_string_view(simple_string_view sv)
        {
            return string_view{ sv.data, sv.length };
        }

        template<typename T>
        struct any_arg_impl : any_arg
        {
            any_arg_impl(T const& value) : m_value(value) {}

            bool parse(simple_string_view specifier, context& ctx) override
            {
                return m_formatter.parse(to_string_view(specifier), ctx);
            }

            bool try_get_int(int& value) const override
            {
                // TODO use "is convertible to int" trait
                if constexpr (is_integer_v<T>)
                {
                    value = static_cast<int>(m_value);
                    return true;
                }

                return false;
            }

            bool format(context& ctx) const override
            {
                return m_formatter.format(m_value, ctx);
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

    inline bool vformat_to(writer& writer, string_view fmt, args_list const& args)
    {
        context ctx{ writer, args };
        return detail::vformat_to(detail::simple_string_view{ fmt.data(), fmt.length() }, ctx);
    }

    template<typename... Ts>
    bool format_to(writer& writer, string_view fmt, Ts const&... args)
    {
        return vformat_to(writer, fmt, args_list{ args... });
    }
}

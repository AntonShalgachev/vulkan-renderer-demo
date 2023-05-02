#pragma once

namespace picofmt
{
    namespace detail
    {
        struct simple_writer;
        struct simple_string_view;

        struct any_arg
        {
            virtual ~any_arg() = default;
            virtual bool parse(simple_string_view specifier, simple_writer const& ctx) = 0;
            virtual bool format(simple_writer const& ctx) const = 0;
        };

        bool parse_int(simple_string_view str, int& value);
    }

    class args_list
    {
    public:
        template<typename... Ts> args_list(Ts const& ... args); // implemented in core.h because it depends on the user-specified type
        ~args_list();

        detail::any_arg& operator[](size_t index) const;

    private:
        size_t const m_size = 0;
        detail::any_arg** const m_args = nullptr;
    };

    namespace detail
    {
        bool vformat_to(simple_writer const& ctx, simple_string_view fmt, args_list const& args);
    }
}

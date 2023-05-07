#pragma once

namespace picofmt
{
    class args_list;
    class context;

    namespace detail
    {
        struct context_base;
        struct simple_string_view;

        struct any_arg
        {
            virtual ~any_arg() = default;
            virtual bool parse(simple_string_view specifier, context& ctx) = 0;
            virtual bool try_get_int(int& value) const = 0;
            virtual bool format(context& ctx) const = 0;
        };

        bool vformat_to(simple_string_view fmt, context_base& ctx);
    }

    class args_list
    {
        friend bool detail::vformat_to(detail::simple_string_view fmt, detail::context_base& ctx);

    public:
        template<typename... Ts> args_list(Ts const& ... args); // implemented in core.h because it depends on the user-specified type
        ~args_list();

        size_t size() const { return m_size; }
        bool try_get_int(size_t index, int& value) const;

    private:
        detail::any_arg& operator[](size_t index) const;

    private:
        size_t const m_size = 0;
        detail::any_arg** const m_args = nullptr;
    };
}

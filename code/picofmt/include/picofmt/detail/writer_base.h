#pragma once

namespace picofmt
{
    namespace detail
    {
        struct simple_string_view;

        class writer_base
        {
        public:
            virtual ~writer_base() = default;

            [[nodiscard]] virtual bool write(simple_string_view str) = 0;
            [[nodiscard]] virtual bool write(char c, size_t count) = 0;
            virtual void report_error(simple_string_view str) = 0;
        };
    }
}

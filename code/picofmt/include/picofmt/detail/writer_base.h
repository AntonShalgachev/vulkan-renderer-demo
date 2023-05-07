#pragma once

namespace picofmt
{
    struct writer;

    namespace detail
    {
        struct simple_string_view;

        struct writer_base
        {
            virtual ~writer_base() = default;

            [[nodiscard]] virtual bool write(simple_string_view str) = 0;
            [[nodiscard]] virtual bool write(char c, size_t count) = 0;
            virtual void report_error(simple_string_view str) = 0;
        };
    }
}

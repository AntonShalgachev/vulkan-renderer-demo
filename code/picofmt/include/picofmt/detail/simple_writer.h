#pragma once

#include "picofmt/detail/simple_string_view.h"

// TODO rename?

namespace picofmt
{
    struct writer;

    namespace detail
    {
        struct simple_writer
        {
            virtual bool write(simple_string_view str) const = 0;
            virtual void report_error(simple_string_view str) const = 0;
            virtual writer& get_user_writer() const = 0;
        };
    }
}

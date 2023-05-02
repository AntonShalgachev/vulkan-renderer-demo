#pragma once

#include "picofmt/writer.h"

#include "picofmt/detail/simple_writer.h"
#include "picofmt/detail/simple_string_view.h"

namespace picofmt::detail
{
    struct writer_adapter : public simple_writer
    {
        writer_adapter(writer& ctx) : ctx(ctx) {}

        bool write(simple_string_view str) const override
        {
            return ctx.write({ str.data(), str.size() });
        }

        void report_error(simple_string_view str) const override
        {
            return ctx.report_error({ str.data(), str.size() });
        }

        writer& get_user_writer() const override
        {
            return ctx;
        }

        writer& ctx;
    };
}

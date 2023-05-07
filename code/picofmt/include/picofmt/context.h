#pragma once

#include "picofmt/config.h"
#include "picofmt/writer.h"

#include "picofmt/detail/context_base.h"

namespace picofmt
{
    // TODO split into parse_context and format_context?
    class context final : public detail::context_base
    {
    public:
        context(writer& writer, args_list const& args) : detail::context_base(writer, args), writer(writer) {}

        bool write(string_view str) { return writer.write(str); }
        bool write(char c, size_t count) { return writer.write(c, count); }
        void report_error(string_view str) { return writer.report_error(str); }

        context& get_user_context() override { return *this; }

        writer& writer;
    };
}

#pragma once

#include "picofmt/config.h"

#include "picofmt/detail/writer_base.h"
#include "picofmt/detail/simple_string_view.h"

namespace picofmt
{
    // TODO rename to io_handler or something similar?

    class writer : public detail::writer_base
    {
    public:
        virtual bool write(string_view str) = 0;
        virtual void report_error(string_view str) = 0;

        bool write(char c, size_t count) override
        {
            string_view fill{ &c, 1 };
            for (size_t i = 0; i < count; i++)
                if (!write(fill))
                    return false;
            return true;
        }

    private:
        [[nodiscard]] bool write(detail::simple_string_view str) override { return write(string_view{ str.data, str.length }); }
        void report_error(detail::simple_string_view str) override { return report_error(string_view{ str.data, str.length }); }
    };
}

#pragma once

#include "nstl/string.h"
#include "nstl/string_view.h"

#define PICOFMT_CUSTOM_STRING_VIEW nstl::string_view
#include "picofmt/picofmt.h"

// TODO move somewhere
template<>
struct picofmt::formatter<nstl::string_view>
{
    bool parse(string_view specifier, writer& ctx)
    {
        return true;
    }

    bool format(nstl::string_view const& value, writer& ctx) const
    {
        // TODO take format into consideration
        ctx.write(value);
        return true;
    }
};

template<>
struct picofmt::formatter<nstl::string> : public picofmt::formatter<nstl::string_view> {};

namespace common
{
    struct StringAppender : public picofmt::writer
    {
        StringAppender(nstl::string& str) : m_str(&str) {}

        bool write(nstl::string_view str) override
        {
            *m_str += str;
            return true;
        }

        void report_error(nstl::string_view str) override
        {
            assert(false);
        }

    private:
        nstl::string* m_str;
    };

    template<typename... Ts>
    nstl::string format(char const* format, Ts&&... args)
    {
        nstl::string result;

        StringAppender ctx{ result };
        picofmt::format_to(ctx, format, nstl::forward<Ts>(args)...);

        return result;
    }
}

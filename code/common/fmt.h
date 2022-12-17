#pragma once

#include "nstl/string.h"
#include "nstl/string_view.h"

#include "fmt/core.h"

// TODO move somewhere
template<>
struct fmt::formatter<nstl::string_view>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.end();
    }

    template<typename FormatContext>
    auto format(nstl::string_view const& str, FormatContext& ctx)
    {
        // TODO probably can be copied to the iterator directly
        auto it = ctx.out();
        for (char c : str)
            *it++ = c;
        return it;
    }
};

template<>
struct fmt::formatter<nstl::string> : fmt::formatter<nstl::string_view>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return fmt::formatter<nstl::string_view>::parse(ctx);
    }

    template<typename FormatContext>
    auto format(nstl::string const& str, FormatContext& ctx)
    {
        return fmt::formatter<nstl::string_view>::format(str, ctx);
    }
};

namespace common
{
    struct StringAppender
    {
        StringAppender(nstl::string& str) : m_str(&str) {}

        StringAppender& operator=(char c)
        {
            m_str->push_back(c);
            return *this;
        }

        StringAppender& operator*()
        {
            return *this;
        }

        StringAppender& operator++()
        {
            return *this;
        }

        StringAppender operator++(int)
        {
            return *this;
        }

    private:
        nstl::string* m_str;
    };

    template<typename... Ts>
    nstl::string format(char const* format, Ts&&... args)
    {
        nstl::string result;

        fmt::format_to(StringAppender{ result }, fmt::runtime(format), nstl::forward<Ts>(args)...);

        return result;
    }
}

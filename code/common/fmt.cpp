#include "common/fmt.h"

namespace
{
    struct StringAppender : public picofmt::writer
    {
        StringAppender(nstl::string& str) : m_str(str) {}

        bool write(nstl::string_view str) override
        {
            m_str += str;
            return true;
        }

        void report_error(nstl::string_view) override
        {
            // TODO make use of the error message
            assert(false);
        }

    private:
        nstl::string& m_str;
    };
}

bool common::detail::string_case_formatter::parse(nstl::string_view specifier, picofmt::context& ctx)
{
    if (!specifier.empty() && specifier[specifier.length() - 1] == '!')
    {
        upper_case = true;
        specifier = specifier.substr(0, specifier.length() - 1);
    }

    return picofmt::formatter<nstl::string_view>::parse(specifier, ctx);
}

bool common::detail::string_case_formatter::format(nstl::string_view const& value, picofmt::context& ctx) const
{
    if (upper_case)
    {
        nstl::string uppercase_copy = value;
        for (char& c : uppercase_copy)
            c = static_cast<char>(toupper(c));

        return picofmt::formatter<nstl::string_view>::format(uppercase_copy, ctx);
    }

    return picofmt::formatter<nstl::string_view>::format(value, ctx);
}

nstl::string common::vformat(nstl::string_view format, picofmt::args_list const& args)
{
    nstl::string result;

    StringAppender ctx{ result };
    picofmt::vformat_to(ctx, format, args);

    return result;
}

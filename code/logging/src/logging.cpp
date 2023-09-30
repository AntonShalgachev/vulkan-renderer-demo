#include "logging/logging.h"

#include "platform/debug.h"

#include "nstl/vector.h"
#include "nstl/unique_ptr.h"

#include "stdio.h"

namespace
{
    struct log_writer : public picofmt::writer
    {
        bool write(nstl::string_view str) override
        {
            if (str.empty())
                return true;

            platform::debug_output(str);

            return true;
        }

        void report_error(nstl::string_view) override
        {
            // TODO replace with something that would crash in Release
            // TODO make use of the error string
            assert(false);
        }
    };
}

namespace logging
{
    void log(level level, nstl::string_view str, nstl::source_location loc)
    {
        MEMORY_TRACKING_SCOPE(get_scope_id());

        log_writer out{};
        bool res = picofmt::format_to(out, "{}({},{}): [{:!}] {}\n", loc.file_name(), loc.line(), loc.column(), level, str);
        assert(res);
    }

    void vlogf(level level, nstl::string_view format, picofmt::args_list const& args, nstl::source_location loc)
    {
        MEMORY_TRACKING_SCOPE(get_scope_id());
        return log(level, common::vformat(format, args), loc);
    }
}

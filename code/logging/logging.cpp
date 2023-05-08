#include "logging.h"

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

        void report_error(nstl::string_view str) override
        {
            assert(false);
        }
    };
}

namespace logging
{
    void log(level level, nstl::string_view str)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        log_writer out{};
        bool res = picofmt::format_to(out, "[{:!}] {}\n", level, str);
        assert(res);
    }

    void vlog(level level, nstl::string_view format, picofmt::args_list const& args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return log(level, common::vformat(format, args));
    }
}

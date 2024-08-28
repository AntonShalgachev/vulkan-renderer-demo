#include "logging/logging.h"

#include "platform/debug.h"

#include "nstl/vector.h"
#include "nstl/unique_ptr.h"

#include "stdio.h"

namespace logging
{
    void log(level level, nstl::string_view str, nstl::source_location loc)
    {
        MEMORY_TRACKING_SCOPE(get_scope_id());

        nstl::string result = common::format("{}({},{}): [{:!}] {}\n", loc.file_name(), loc.line(), loc.column(), level, str);
        platform::debug_output(result);
    }

    void vlogf(level level, nstl::string_view format, picofmt::args_list const& args, nstl::source_location loc)
    {
        MEMORY_TRACKING_SCOPE(get_scope_id());
        return log(level, common::vformat(format, args), loc);
    }
}

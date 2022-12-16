#include "logging.h"

namespace
{
    void log(char const* tag, nstl::string_view str)
    {
        // TODO colors and various destinations
        printf("[%s] %.*s\n", tag, str.slength(), str.data());
    }
}

namespace logging
{
    void info(nstl::string_view str)
    {
        log("INFO", str);
    }

    void warn(nstl::string_view str)
    {
        log("WARN", str);
    }

    void error(nstl::string_view str)
    {
        log("ERROR", str);
    }
}

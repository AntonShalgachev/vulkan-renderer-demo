#pragma once

#include "common/fmt.h"
#include "common/tiny_ctti.h"

#include "memory/tracking.h"

#include "nstl/string_view.h"

namespace logging
{
    class sink
    {
    public:
        virtual ~sink() = default;
        virtual void log(nstl::string_view str) = 0;
    };
}

namespace logging
{
    inline auto scope_id = memory::tracking::create_scope_id("System/Logging");

    enum class level
    {
        info,
        warn,
        error,
    };
    TINY_CTTI_DESCRIBE_ENUM(level, info, warn, error);

    void log(level level, nstl::string_view str);

    void vlog(level level, nstl::string_view format, picofmt::args_list const& args);

    template<picofmt::formattable... Ts>
    void log(level level, nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlog(level, format, { args... });
    }

    template<picofmt::formattable... Ts>
    void info(nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlog(level::info, format, { args... });
    }

    template<picofmt::formattable... Ts>
    void warn(nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlog(level::warn, format, { args... });
    }

    template<picofmt::formattable... Ts>
    void error(nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlog(level::error, format, { args... });
    }
}

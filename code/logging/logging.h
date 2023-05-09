#pragma once

#include "common/fmt.h"
#include "common/tiny_ctti.h"

#include "memory/tracking.h"

#include "nstl/string_view.h"
#include "nstl/source_location.h"

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

    struct format_with_location
    {
        format_with_location(nstl::string_view format, nstl::source_location location = {}) : format(format), location(location) {}
        format_with_location(char const* format, nstl::source_location location = {}) : format_with_location(nstl::string_view{format}, location) {}

        nstl::string_view format;
        nstl::source_location location;
    };

    void log(level level, nstl::string_view str, nstl::source_location loc = {});

    void vlogf(level level, nstl::string_view format, picofmt::args_list const& args, nstl::source_location loc = {});

    template<picofmt::formattable... Ts>
    void logf(level level, nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlogf(level, format, { args... });
    }

    template<picofmt::formattable... Ts>
    void info(format_with_location format_location, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlogf(level::info, format_location.format, { args... }, format_location.location);
    }

    template<picofmt::formattable... Ts>
    void warn(nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlogf(level::warn, format, { args... });
    }

    template<picofmt::formattable... Ts>
    void error(nstl::string_view format, Ts const&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return vlogf(level::error, format, { args... });
    }
}

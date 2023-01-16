#pragma once

#include "common/fmt.h"

#include "memory/tracking.h"

#include "nstl/string_view.h"

namespace logging
{
    inline auto scope_id = memory::tracking::create_scope_id("System/Logging");

    void info(nstl::string_view str);
    void warn(nstl::string_view str);
    void error(nstl::string_view str);

    template<typename... Ts>
    void info(char const* format, Ts&&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return info(common::format(format, nstl::forward<Ts>(args)...));
    }

    template<typename... Ts>
    void warn(char const* format, Ts&&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return warn(common::format(format, nstl::forward<Ts>(args)...));
    }

    template<typename... Ts>
    void error(char const* format, Ts&&... args)
    {
        MEMORY_TRACKING_SCOPE(scope_id);
        return error(common::format(format, nstl::forward<Ts>(args)...));
    }
}

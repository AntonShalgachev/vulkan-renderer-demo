#pragma once

#include "common/fmt.h"

#include "nstl/string_view.h"

namespace logging
{
    void info(nstl::string_view str);
    void warn(nstl::string_view str);
    void error(nstl::string_view str);

    template<typename... Ts>
    void info(char const* format, Ts&&... args)
    {
        return info(common::format(format, nstl::forward<Ts>(args)...));
    }

    template<typename... Ts>
    void warn(char const* format, Ts&&... args)
    {
        return warn(common::format(format, nstl::forward<Ts>(args)...));
    }

    template<typename... Ts>
    void error(char const* format, Ts&&... args)
    {
        return error(common::format(format, nstl::forward<Ts>(args)...));
    }
}

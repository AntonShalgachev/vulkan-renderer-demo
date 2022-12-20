#pragma once

#include "nstl/string.h"

namespace vko
{
    struct DebugMessage
    {
        enum class Level
        {
            Info,
            Warning,
            Error,
        };

        Level level;
        nstl::string id;
        nstl::string text;
    };
}

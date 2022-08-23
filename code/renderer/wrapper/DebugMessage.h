#pragma once

#include <string>

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
        std::string id;
        std::string text;
    };
}

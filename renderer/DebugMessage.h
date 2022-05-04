#pragma once

#include <string>

namespace vkr
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

#pragma once

#include "nstl/vector.h"

#include <string_view>
#include <string>

namespace vkc
{
    namespace utils
    {
        template<typename StringVector1, typename StringVector2>
        bool hasEveryOption(StringVector1 const& availableOptions, StringVector2 const& requestedOptions)
        {
            for (const auto& requestedOption : requestedOptions)
            {
                auto it = std::find_if(availableOptions.begin(), availableOptions.end(), [requestedOption](auto const& availableOption)
                {
                    return std::string_view{ availableOption } == std::string_view{ requestedOption };
                });

                if (it == availableOptions.end())
                    return false;
            }

            return true;
        }

        // TODO implement asset manager instead
        nstl::vector<unsigned char> readFile(const std::string& filename);
    }
}

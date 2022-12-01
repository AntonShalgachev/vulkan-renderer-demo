#pragma once

#include "nstl/vector.h"
#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/algorithm.h"

namespace vkc
{
    namespace utils
    {
        template<typename StringVector1, typename StringVector2>
        bool hasEveryOption(StringVector1 const& availableOptions, StringVector2 const& requestedOptions)
        {
            for (const auto& requestedOption : requestedOptions)
            {
                auto it = nstl::find_if(availableOptions.begin(), availableOptions.end(), [requestedOption](auto const& availableOption)
                {
                    return nstl::string_view{ availableOption } == nstl::string_view{ requestedOption };
                });

                if (it == availableOptions.end())
                    return false;
            }

            return true;
        }

        // TODO implement asset manager instead
        nstl::vector<unsigned char> readFile(char const* filename);
        nstl::string readTextFile(char const* filename);

        nstl::vector<nstl::string_view> split(nstl::string_view str);
    }
}

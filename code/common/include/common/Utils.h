#pragma once

#include "nstl/vector.h"
#include "nstl/string_view.h"
#include "nstl/span.h"
#include "nstl/string.h"
#include "nstl/algorithm.h"
#include "nstl/blob.h"

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

        template<typename StringVector1, typename StringVector2, typename TransformFunc>
        bool hasEveryOption(StringVector1 const& availableOptions, StringVector2 const& requestedOptions, TransformFunc&& getAvailableOptionString)
        {
            for (const auto& requestedOption : requestedOptions)
            {
                auto it = nstl::find_if(availableOptions.begin(), availableOptions.end(), [&requestedOption, &getAvailableOptionString](auto const& availableOption)
                {
                    auto&& name = getAvailableOptionString(availableOption);
                    return nstl::string_view{ name } == nstl::string_view{ requestedOption };
                });

                if (it == availableOptions.end())
                    return false;
            }

            return true;
        }

        // TODO implement asset manager instead
        nstl::blob readBinaryFile(nstl::string_view filename);

        // TODO implement as an interator
        nstl::vector<nstl::string_view> split(nstl::string_view str);
    }
}

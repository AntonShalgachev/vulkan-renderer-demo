#include "common/Utils.h"

#include "nstl/blob.h"

#include <assert.h>

nstl::vector<nstl::string_view> vkc::utils::split(nstl::string_view str)
{
    nstl::vector<nstl::string_view> result;
    nstl::string_view rest = str;
    
    while (!rest.empty())
    {
        auto pos = rest.find_first_of("\n\r");
        if (pos == nstl::string_view::npos)
            break;

        result.push_back(rest.substr(0, pos));

        bool hasMoreChars = pos < rest.size() - 1;
        bool isNextCR = hasMoreChars && rest[pos] == '\n' && rest[pos + 1] == '\r';
        bool isNextLF = hasMoreChars && rest[pos] == '\r' && rest[pos + 1] == '\n';

        size_t charsToSkip = 1;
        if (isNextCR || isNextLF)
            charsToSkip++;

        rest = rest.substr(pos + charsToSkip);
    }

    result.push_back(rest);

    return result;
}

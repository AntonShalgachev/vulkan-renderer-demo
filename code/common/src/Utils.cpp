#include "common/Utils.h"

#include "memory/tracking.h"

#include "nstl/blob.h"

#include "fs/file.h"

#include <assert.h>

namespace
{
    auto fileScopeId = memory::tracking::create_scope_id("IO/ReadFile");
}

nstl::blob vkc::utils::readBinaryFile(nstl::string_view filename)
{
    MEMORY_TRACKING_SCOPE(fileScopeId);

    fs::file f{ filename, fs::open_mode::read };

    nstl::blob buffer{ f.size() };
    f.read(buffer.data(), buffer.size());

    return buffer;
}

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

        auto charsToSkip = 1;
        if (isNextCR || isNextLF)
            charsToSkip++;

        rest = rest.substr(pos + charsToSkip);
    }

    result.push_back(rest);

    return result;
}

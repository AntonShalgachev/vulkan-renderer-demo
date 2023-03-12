#include "platform/filesystem.h"

#include "nstl/string.h"

#include "Windows.h"

void platform::create_directory(nstl::string_view path)
{
    nstl::string pathCopy = path;

    bool result = CreateDirectoryA(pathCopy.c_str(), nullptr);

    auto lastError = GetLastError();
    if (!result)
        assert(GetLastError() == ERROR_ALREADY_EXISTS);
}

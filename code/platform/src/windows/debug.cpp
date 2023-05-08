#include "platform/debug.h"

#include "windows_common.h"

#include "nstl/string.h"

void platform::debug_output(nstl::string_view str)
{
    if (str.empty())
        return;

    nstl::string copy = str;
    OutputDebugStringA(copy.c_str());
}

#include "nstl/sprintf.h"

#include <stdio.h>

nstl::string nstl::sprintf(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    auto res = vsprintf(format, args);
    va_end(args);

    return res;
}

nstl::string nstl::vsprintf(char const* format, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    size_t stringSize = static_cast<size_t>(vsnprintf(nullptr, 0, format, args2));
    va_end(args2);

    string str;
    str.resize(stringSize);

    [[maybe_unused]] int result = vsnprintf(str.data(), str.length() + 1, format, args);

    NSTL_ASSERT(result >= 0 && static_cast<size_t>(result) <= str.length());

    return str;
}

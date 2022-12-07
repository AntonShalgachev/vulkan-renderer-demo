#pragma once

#include "nstl/config.h"
#include "nstl/string.h"

#include <stdarg.h>

namespace nstl
{
    NSTL_PRINTF_LIKE(1, 2) string sprintf(char const* format, ...);
    string vsprintf(char const* format, va_list args);
}
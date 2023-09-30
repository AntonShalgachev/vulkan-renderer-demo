#pragma once

#include "nstl/config.h"
#include "nstl/string.h"

#include <stdarg.h>

namespace nstl
{
    NSTL_PRINTF_LIKE(1, 2) string sprintf(char const* format, ...);
    NSTL_PRINTF_LIKE(1, 0) string vsprintf(char const* format, va_list args);
}

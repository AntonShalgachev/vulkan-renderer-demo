#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <stdint.h>

#include "nstl/string.h"

namespace platform_win64
{
    struct error
    {
        DWORD code;
        nstl::string message;
    };

    error get_last_error();
}

#pragma once

#include <Windows.h>

#include <stdint.h>

#include "nstl/string.h"

namespace platform_win32
{
    uint64_t create_handle(HANDLE h);
    HANDLE get_handle(uint64_t handle);

    struct error
    {
        DWORD code;
        nstl::string message;
    };

    error get_last_error();
}

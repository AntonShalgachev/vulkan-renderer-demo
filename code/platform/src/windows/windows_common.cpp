#include "windows_common.h"

#include <assert.h>

uint64_t platform_win32::create_handle(HANDLE h)
{
    assert(h != INVALID_HANDLE_VALUE);

    uint64_t handle;

    static_assert(sizeof(handle) >= sizeof(h));
    memcpy(&handle, &h, sizeof(h));

    return handle;
}

HANDLE platform_win32::get_handle(uint64_t handle)
{
    HANDLE h;

    static_assert(sizeof(handle) >= sizeof(h));
    memcpy(&h, &handle, sizeof(h));

    assert(h != INVALID_HANDLE_VALUE);

    return h;
}

platform_win32::error platform_win32::get_last_error()
{
    auto code = GetLastError();

    char* buffer = nullptr;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&buffer,
        0,
        nullptr
    );

    error e;
    e.code = code;
    e.message = buffer;

    LocalFree(buffer);

    return e;
}

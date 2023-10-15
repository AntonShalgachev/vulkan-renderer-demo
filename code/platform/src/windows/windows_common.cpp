#include "windows_common.h"

platform_win32::error platform_win32::get_last_error()
{
    auto code = GetLastError();

    char* buffer = nullptr;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<char*>(&buffer),
        0,
        nullptr
    );

    error e;
    e.code = code;
    e.message = buffer;

    LocalFree(buffer);

    return e;
}

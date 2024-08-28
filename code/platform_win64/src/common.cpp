#include "common.h"

#include "nstl/string_view.h"

#include <limits.h>

platform_win64::error platform_win64::get_last_error()
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

platform_win64::wstring platform_win64::convert_to_wstring(nstl::string_view str)
{
    if (str.empty())
        return {};

    assert(str.size() <= INT_MAX);
    int length = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    assert(length > 0);

    wstring result{ static_cast<size_t>(length) + 1 };

    assert(result.length() <= INT_MAX);
    int chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), static_cast<int>(result.length()));
    assert(chars_written == length);

    result[chars_written] = 0;

    return result;
}

nstl::string platform_win64::convert_to_string(WCHAR const* str)
{
    int length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    if (length == 0)
        return {};

    nstl::string result{ static_cast<size_t>(length) };

    assert(result.length() <= INT_MAX);
    int chars_written = WideCharToMultiByte(CP_UTF8, 0, str, -1, result.data(), static_cast<int>(result.length()), nullptr, nullptr);
    assert(chars_written == length);

    return result;
}

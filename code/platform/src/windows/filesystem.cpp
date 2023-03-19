#include "platform/filesystem.h"

#include "windows_common.h"

#include "fs/file.h"

#include "nstl/string.h"

#include <Windows.h>

void platform::create_directory(nstl::string_view path)
{
    nstl::string pathCopy = path;

    bool result = CreateDirectoryA(pathCopy.c_str(), nullptr);

    auto lastError = GetLastError();
    if (!result)
        assert(GetLastError() == ERROR_ALREADY_EXISTS);
}

nstl::optional<platform::file_handle> platform::open_file(nstl::string_view filename, fs::open_mode mode)
{
    assert(filename.length() <= MAX_PATH);
    nstl::string filenameCopy = filename;

    DWORD desiredAccess = 0; // GENERIC_READ, GENERIC_WRITE
    DWORD shareMode = 0;
    DWORD creationDisposition = 0; // OPEN_EXISTING, CREATE_ALWAYS
    DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

    if (mode == fs::open_mode::read)
    {
        desiredAccess = GENERIC_READ;
        creationDisposition = OPEN_EXISTING;
    }
    else if (mode == fs::open_mode::write)
    {
        desiredAccess = GENERIC_WRITE;
        creationDisposition = CREATE_ALWAYS;
    }
    else
    {
        assert(false);
    }

    HANDLE h = CreateFileA(filenameCopy.c_str(), desiredAccess, shareMode, nullptr, creationDisposition, flagsAndAttributes, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return {};
    }

    return platform_win32::create_handle(h);
}

void platform::close_file(file_handle handle)
{
    if (!CloseHandle(platform_win32::get_handle(handle)))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
    }
}

size_t platform::get_file_size(file_handle handle)
{
    LARGE_INTEGER size;
    if (!GetFileSizeEx(platform_win32::get_handle(handle), &size))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
    }

    assert(size.QuadPart >= 0);
    return static_cast<size_t>(size.QuadPart);
}

bool platform::read_file(file_handle handle, void* data, size_t size, size_t offset)
{
    OVERLAPPED o{};
    o.Offset = offset;

    DWORD bytesRead = 0;
    if (!ReadFile(platform_win32::get_handle(handle), data, size, &bytesRead, &o))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytesRead == size;
}

bool platform::write_file(file_handle handle, void const* data, size_t size, size_t offset)
{
    OVERLAPPED o{};
    o.Offset = offset;

    DWORD bytesWritten = 0;
    if (!WriteFile(platform_win32::get_handle(handle), data, size, &bytesWritten, &o))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytesWritten == size;
}

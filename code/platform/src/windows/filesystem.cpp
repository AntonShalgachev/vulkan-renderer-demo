#include "platform/filesystem.h"

#include "windows_common.h"

#include "fs/file.h"

#include "nstl/string.h"

void platform::create_directory(nstl::string_view path)
{
    nstl::string pathCopy = path;

    bool result = CreateDirectoryA(pathCopy.c_str(), nullptr);

    auto lastError = GetLastError();
    if (!result)
        assert(GetLastError() == ERROR_ALREADY_EXISTS);
}

bool platform::open_file(file_storage_t& storage, nstl::string_view filename, fs::open_mode mode)
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
        return false;
    }

    storage.create<HANDLE>(h);
    return true;
}

void platform::close_file(file_storage_t& storage)
{
    HANDLE handle = storage.get_as<HANDLE>();
    if (!CloseHandle(handle))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
    }
}

size_t platform::get_file_size(file_storage_t& storage)
{
    HANDLE handle = storage.get_as<HANDLE>();

    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
    }

    assert(size.QuadPart >= 0);
    return static_cast<size_t>(size.QuadPart);
}

bool platform::read_file(file_storage_t& storage, void* data, size_t size, size_t offset)
{
    HANDLE handle = storage.get_as<HANDLE>();

    OVERLAPPED o{};
    o.Offset = offset;

    DWORD bytesRead = 0;
    if (!ReadFile(handle, data, size, &bytesRead, &o))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytesRead == size;
}

bool platform::write_file(file_storage_t& storage, void const* data, size_t size, size_t offset)
{
    HANDLE handle = storage.get_as<HANDLE>();

    OVERLAPPED o{};
    o.Offset = offset;

    DWORD bytesWritten = 0;
    if (!WriteFile(handle, data, size, &bytesWritten, &o))
    {
        auto e = platform_win32::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytesWritten == size;
}

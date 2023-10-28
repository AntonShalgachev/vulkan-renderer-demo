#include "platform/filesystem.h"

#include "common.h"

#include "fs/file.h"

#include "nstl/string.h"

void platform::create_directory(nstl::string_view path)
{
    nstl::string path_copy = path;

    bool result = CreateDirectoryA(path_copy.c_str(), nullptr);

    [[maybe_unused]] auto lastError = GetLastError();
    if (!result)
        assert(lastError == ERROR_ALREADY_EXISTS);
}

bool platform::open_file(file_storage_t& storage, nstl::string_view filename, fs::open_mode mode)
{
    assert(filename.length() <= MAX_PATH);
    nstl::string filename_copy = filename;

    DWORD desired_access = 0; // GENERIC_READ, GENERIC_WRITE
    DWORD share_mode = 0;
    DWORD creation_disposition = 0; // OPEN_EXISTING, CREATE_ALWAYS
    DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;

    if (mode == fs::open_mode::read)
    {
        desired_access = GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
    }
    else if (mode == fs::open_mode::write)
    {
        desired_access = GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    }
    else
    {
        assert(false);
    }

    HANDLE h = CreateFileA(filename_copy.c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        auto e = platform_win64::get_last_error(); // TODO make use of it
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
        auto e = platform_win64::get_last_error(); // TODO make use of it
        assert(false);
    }
}

size_t platform::get_file_size(file_storage_t& storage)
{
    HANDLE handle = storage.get_as<HANDLE>();

    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size))
    {
        auto e = platform_win64::get_last_error(); // TODO make use of it
        assert(false);
    }

    assert(size.QuadPart >= 0);
    return static_cast<size_t>(size.QuadPart);
}

bool platform::read_file(file_storage_t& storage, void* data, size_t size, size_t offset)
{
    HANDLE handle = storage.get_as<HANDLE>();

    OVERLAPPED o{};
    assert(offset <= UINT32_MAX);
    o.Offset = static_cast<uint32_t>(offset);

    assert(size <= UINT32_MAX);

    DWORD bytes_read = 0;
    if (!ReadFile(handle, data, static_cast<uint32_t>(size), &bytes_read, &o))
    {
        auto e = platform_win64::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytes_read == size;
}

bool platform::write_file(file_storage_t& storage, void const* data, size_t size, size_t offset)
{
    HANDLE handle = storage.get_as<HANDLE>();

    OVERLAPPED o{};
    assert(offset <= UINT32_MAX);
    o.Offset = static_cast<uint32_t>(offset);

        assert(size <= UINT32_MAX);

    DWORD bytes_written = 0;
    if (!WriteFile(handle, data, static_cast<uint32_t>(size), &bytes_written, &o))
    {
        auto e = platform_win64::get_last_error(); // TODO make use of it
        assert(false);
        return false;
    }

    return bytes_written == size;
}

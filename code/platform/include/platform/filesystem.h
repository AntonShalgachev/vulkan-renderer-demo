#pragma once

#include "nstl/string_view.h"
#include "nstl/optional.h"

#include <stdint.h>

namespace fs
{
    enum class open_mode;
}

namespace platform
{
    using file_handle = uint64_t;

    void create_directory(nstl::string_view path);

    nstl::optional<file_handle> open_file(nstl::string_view filename, fs::open_mode mode);
    void close_file(file_handle handle);
    size_t get_file_size(file_handle handle);
    bool read_file(file_handle handle, void* data, size_t size, size_t offset);
    bool write_file(file_handle handle, void const* data, size_t size, size_t offset);
}

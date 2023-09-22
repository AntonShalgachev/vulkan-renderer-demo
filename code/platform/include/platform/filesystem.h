#pragma once

#include "nstl/aligned_storage.h"
#include "nstl/string_view.h"
#include "nstl/optional.h"

#include <stdint.h>

namespace fs
{
    enum class open_mode;
}

namespace platform
{
    using file_storage_t = nstl::aligned_storage_t<8, 8>;

    void create_directory(nstl::string_view path);

    [[nodiscard]] bool open_file(file_storage_t& storage, nstl::string_view filename, fs::open_mode mode);
    void close_file(file_storage_t& storage);
    [[nodiscard]] size_t get_file_size(file_storage_t& storage);
    [[nodiscard]] bool read_file(file_storage_t& storage, void* data, size_t size, size_t offset);
    [[nodiscard]] bool write_file(file_storage_t& storage, void const* data, size_t size, size_t offset);
}

#pragma once

#include "platform/filesystem.h"

#include "nstl/string_view.h"

namespace fs
{
    enum class open_mode
    {
        read,
        write,
    };

    class file
    {
    public:
        file();
        file(nstl::string_view filename, open_mode mode);
        ~file();

        [[nodiscard]] bool try_open(nstl::string_view filename, open_mode mode);
        void open(nstl::string_view filename, open_mode mode);
        void close();

        bool is_open() const;
        size_t size() const;

        [[nodiscard]] bool try_read(void* data, size_t size, size_t offset = 0) const;
        void read(void* data, size_t size, size_t offset = 0) const;
        [[nodiscard]] bool try_write(void const* data, size_t size, size_t offset = 0) const;
        void write(void const* data, size_t size, size_t offset = 0) const;

    private:
        nstl::optional<platform::file_handle> m_handle;
    };
}

#include "fs/file.h"

fs::file::file() = default;

fs::file::file(nstl::string_view filename, open_mode mode)
{
    open(filename, mode);
}

fs::file::~file()
{
    close();
}

bool fs::file::try_open(nstl::string_view filename, open_mode mode)
{
    if (is_open())
        return false;

    m_is_open = platform::open_file(m_storage, filename, mode);
    return m_is_open;
}

void fs::file::open(nstl::string_view filename, open_mode mode)
{
    bool res = try_open(filename, mode);
    assert(res);
    assert(m_is_open);
}

void fs::file::close()
{
    if (!is_open())
        return;

    platform::close_file(m_storage);
    m_is_open = false;
}

bool fs::file::is_open() const
{
    return m_is_open;
}

size_t fs::file::size()
{
    assert(m_is_open);
    return platform::get_file_size(m_storage);
}

bool fs::file::try_read(void* data, size_t size, size_t offset)
{
    assert(m_is_open);
    return platform::read_file(m_storage, data, size, offset);
}

void fs::file::read(void* data, size_t size, size_t offset)
{
    bool res = try_read(data, size, offset);
    assert(res);
}

bool fs::file::try_write(void const* data, size_t size, size_t offset)
{
    assert(m_is_open);
    return platform::write_file(m_storage, data, size, offset);
}

void fs::file::write(void const* data, size_t size, size_t offset)
{
    bool res = try_write(data, size, offset);
    assert(res);
}

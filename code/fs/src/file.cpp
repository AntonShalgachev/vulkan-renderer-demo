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

    m_handle = platform::open_file(filename, mode);
    return m_handle.has_value();
}

void fs::file::open(nstl::string_view filename, open_mode mode)
{
    bool res = try_open(filename, mode);
    assert(res);
    assert(is_open());
}

void fs::file::close()
{
    if (m_handle)
        platform::close_file(*m_handle);
    m_handle = {};
}

bool fs::file::is_open() const
{
    return m_handle.has_value();
}

size_t fs::file::size() const
{
    assert(m_handle);
    return platform::get_file_size(*m_handle);
}

bool fs::file::try_read(void* data, size_t size, size_t offset) const
{
    assert(m_handle);
    return platform::read_file(*m_handle, data, size, offset);
}

void fs::file::read(void* data, size_t size, size_t offset) const
{
    bool res = try_read(data, size, offset);
    assert(res);
}

bool fs::file::try_write(void const* data, size_t size, size_t offset) const
{
    assert(m_handle);
    return platform::write_file(*m_handle, data, size, offset);
}

void fs::file::write(void const* data, size_t size, size_t offset) const
{
    bool res = try_write(data, size, offset);
    assert(res);
}

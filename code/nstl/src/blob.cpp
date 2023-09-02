#include "nstl/blob.h"

#include "nstl/blob_view.h"

nstl::blob::blob(size_t size, any_allocator alloc) : m_buffer(size, sizeof(char), nstl::move(alloc))
{
    m_buffer.resize(size);
}

size_t nstl::blob::size() const
{
    return m_buffer.size();
}

void* nstl::blob::data()
{
    return m_buffer.data();
}

void const* nstl::blob::data() const
{
    return m_buffer.data();
}

char* nstl::blob::cdata()
{
    return m_buffer.data();
}

char const* nstl::blob::cdata() const
{
    return m_buffer.data();
}

unsigned char* nstl::blob::ucdata()
{
    return reinterpret_cast<unsigned char*>(m_buffer.data());
}

unsigned char const* nstl::blob::ucdata() const
{
    return reinterpret_cast<unsigned char const*>(m_buffer.data());
}

nstl::blob::operator nstl::blob_view() const
{
    return { data(), size() };
}

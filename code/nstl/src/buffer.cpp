#include "nstl/buffer.h"

#include <string.h>

nstl::Buffer::Buffer(size_t capacity, size_t chunkSize) : m_ptr(capacity* chunkSize > 0 ? new char[capacity * chunkSize] : nullptr), m_capacity(capacity), m_chunkSize(chunkSize)
{
    NSTL_ASSERT(chunkSize > 0);
}

nstl::Buffer::Buffer(Buffer const& rhs) : Buffer(rhs.m_capacity, rhs.m_chunkSize)
{
    NSTL_ASSERT(rhs.m_constructedObjectsCount == 0);
    copy(rhs.m_ptr, rhs.m_size * m_chunkSize);
    m_size = rhs.m_size;
}

nstl::Buffer::Buffer(Buffer&& rhs)
{
    rhs.swap(*this);
}

nstl::Buffer::~Buffer()
{
    NSTL_ASSERT(m_constructedObjectsCount == 0);

    if (m_ptr)
        delete[] m_ptr;
    m_ptr = nullptr;
}

nstl::Buffer& nstl::Buffer::operator=(Buffer const& rhs)
{
    NSTL_ASSERT(rhs.m_constructedObjectsCount == 0);

    Buffer temp{ rhs };
    temp.swap(*this);
    return *this;
}

nstl::Buffer& nstl::Buffer::operator=(Buffer&& rhs)
{
    rhs.swap(*this);
    return *this;
}

char* nstl::Buffer::data()
{
    return m_ptr;
}

char const* nstl::Buffer::data() const
{
    return m_ptr;
}

size_t nstl::Buffer::capacity() const
{
    return m_capacity;
}

size_t nstl::Buffer::capacityBytes() const
{
    return m_capacity * m_chunkSize;
}

size_t nstl::Buffer::size() const
{
    return m_size;
}

void nstl::Buffer::resize(size_t newSize)
{
    NSTL_ASSERT(newSize <= m_capacity);
    m_size = newSize;
}

char* nstl::Buffer::get(size_t index)
{
    return m_ptr + index;
}

char const* nstl::Buffer::get(size_t index) const
{
    return m_ptr + index;
}

void nstl::Buffer::copy(void const* ptr, size_t size)
{
    if (size == 0)
        return;

    NSTL_ASSERT(ptr);
    NSTL_ASSERT(size <= capacityBytes());

    memcpy(m_ptr, ptr, size);
}

void nstl::Buffer::swap(Buffer& rhs) noexcept
{
    nstl::exchange(m_ptr, rhs.m_ptr);
    nstl::exchange(m_capacity, rhs.m_capacity);
    nstl::exchange(m_chunkSize, rhs.m_chunkSize);

    nstl::exchange(m_size, rhs.m_size);

#if NSTL_CONFIG_ENABLE_ASSERTS
    nstl::exchange(m_constructedObjectsCount, rhs.m_constructedObjectsCount);
#endif
}

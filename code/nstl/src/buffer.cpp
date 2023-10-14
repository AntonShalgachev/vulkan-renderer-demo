#include "nstl/buffer.h"

#include "nstl/algorithm.h"
#include "nstl/malloc_allocator.h"

#include <string.h>

nstl::buffer::buffer(size_t capacity, size_t chunkSize, any_allocator alloc)
    : m_allocator(alloc ? nstl::move(alloc) : malloc_allocator{})
    , m_ptr(capacity*chunkSize > 0 ? static_cast<char*>(m_allocator.allocate(capacity * chunkSize)) : nullptr)
    , m_capacity(capacity)
    , m_chunkSize(chunkSize)
{
    NSTL_ASSERT(chunkSize > 0);
}

nstl::buffer::buffer(buffer const& rhs) : buffer(rhs.m_capacity, rhs.m_chunkSize, rhs.m_allocator)
{
    NSTL_ASSERT(rhs.m_constructedObjectsCount == 0);

    copy(rhs.m_ptr, rhs.m_size);

    m_size = rhs.m_size;
}

nstl::buffer::buffer(buffer&& rhs)
{
    rhs.swap(*this);
}

nstl::buffer::~buffer()
{
    NSTL_ASSERT(m_constructedObjectsCount == 0);

    if (m_ptr)
        m_allocator.deallocate(m_ptr);
    m_ptr = nullptr;
}

nstl::buffer& nstl::buffer::operator=(buffer const& rhs)
{
    NSTL_ASSERT(rhs.m_constructedObjectsCount == 0);
    NSTL_ASSERT(m_allocator == rhs.m_allocator);

    buffer temp{ rhs };
    temp.swap(*this);
    return *this;
}

nstl::buffer& nstl::buffer::operator=(buffer&& rhs)
{
    rhs.swap(*this);
    return *this;
}

size_t nstl::buffer::capacityBytes() const
{
    return m_capacity * m_chunkSize;
}

void nstl::buffer::resize(size_t newSize)
{
    NSTL_ASSERT(newSize <= m_capacity);
    m_size = newSize;
}

char* nstl::buffer::get(size_t index)
{
    return m_ptr + index;
}

char const* nstl::buffer::get(size_t index) const
{
    return m_ptr + index;
}

void nstl::buffer::copy(void const* ptr, size_t size)
{
    if (size == 0)
        return;

    NSTL_ASSERT(ptr);
    NSTL_ASSERT(size * m_chunkSize <= capacityBytes());

    memcpy(m_ptr, ptr, size * m_chunkSize);
}

nstl::any_allocator const& nstl::buffer::get_allocator() const
{
    return m_allocator;
}

void nstl::buffer::swap(buffer& rhs) noexcept
{
    nstl::exchange(m_allocator, rhs.m_allocator);
    nstl::exchange(m_ptr, rhs.m_ptr);
    nstl::exchange(m_capacity, rhs.m_capacity);
    nstl::exchange(m_chunkSize, rhs.m_chunkSize);

    nstl::exchange(m_size, rhs.m_size);

#if NSTL_CONFIG_ENABLE_ASSERTS
    nstl::exchange(m_constructedObjectsCount, rhs.m_constructedObjectsCount);
#endif
}

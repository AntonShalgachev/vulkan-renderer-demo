#include "nstl/allocator.h"

#include "nstl/utility.h"

nstl::any_allocator::any_allocator(any_allocator const& rhs)
    : m_allocator(nullptr)
    , m_destructStorage(rhs.m_destructStorage)
    , m_compare(rhs.m_compare)
    , m_copy(rhs.m_copy)
    , m_allocate(rhs.m_allocate)
    , m_deallocate(rhs.m_deallocate)
{
    if (rhs)
    {
        NSTL_ASSERT(m_copy);
        m_copy(&rhs, this);
    }
}

nstl::any_allocator::any_allocator(any_allocator&& rhs)
{
    any_allocator::swap(*this, rhs);
}

nstl::any_allocator::~any_allocator()
{
    if (m_allocator)
    {
        NSTL_ASSERT(m_destructStorage);
        m_destructStorage(this);
    }
}

nstl::any_allocator& nstl::any_allocator::operator=(any_allocator const&)
{
    // TODO implement
    NSTL_ASSERT(false);

    return *this;
}

nstl::any_allocator& nstl::any_allocator::operator=(any_allocator&& rhs)
{
    any_allocator::swap(*this, rhs);
    return *this;
}

void* nstl::any_allocator::allocate(size_t size, size_t alignment)
{
    NSTL_ASSERT(m_allocate);
    return m_allocate(m_allocator, size, alignment);
}

void nstl::any_allocator::deallocate(void* ptr)
{
    NSTL_ASSERT(m_deallocate);
    return m_deallocate(m_allocator, ptr);
}

nstl::any_allocator::operator bool() const
{
    return m_allocator != nullptr;
}

bool nstl::any_allocator::operator==(any_allocator const& rhs) const
{
    if (!m_allocator && !rhs.m_allocator)
        return true;

    if (m_allocator == rhs.m_allocator)
        return true;

    if (m_allocator && rhs.m_allocator)
        return m_compare(*this, rhs);

    return false;
}

void nstl::any_allocator::swap(any_allocator& lhs, any_allocator& rhs)
{
    nstl::exchange(lhs.m_allocator, rhs.m_allocator);
    nstl::exchange(lhs.m_destructStorage, rhs.m_destructStorage);
    nstl::exchange(lhs.m_compare, rhs.m_compare);
    nstl::exchange(lhs.m_copy, rhs.m_copy);
    nstl::exchange(lhs.m_allocate, rhs.m_allocate);
    nstl::exchange(lhs.m_deallocate, rhs.m_deallocate);
}

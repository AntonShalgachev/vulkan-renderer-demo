#include "nstl/allocator.h"

#include "nstl/utility.h"

nstl::any_allocator::any_allocator(any_allocator const& rhs)
    : m_allocator(nullptr)
    , m_destructStorage(rhs.m_destructStorage)
    , m_copy(rhs.m_copy)
    , m_allocate(rhs.m_allocate)
    , m_deallocate(rhs.m_deallocate)
{
    m_copy(&rhs, this);
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

void* nstl::any_allocator::allocate(size_t size)
{
    NSTL_ASSERT(m_allocate);
    return m_allocate(m_allocator, size);
}

void nstl::any_allocator::deallocate(void* ptr)
{
    NSTL_ASSERT(m_deallocate);
    return m_deallocate(m_allocator, ptr);
}

void nstl::any_allocator::swap(any_allocator& lhs, any_allocator& rhs)
{
    nstl::exchange(lhs.m_allocator, rhs.m_allocator);
    nstl::exchange(lhs.m_destructStorage, rhs.m_destructStorage);
    nstl::exchange(lhs.m_copy, rhs.m_copy);
    nstl::exchange(lhs.m_allocate, rhs.m_allocate);
    nstl::exchange(lhs.m_deallocate, rhs.m_deallocate);
}

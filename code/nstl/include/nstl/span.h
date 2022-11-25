#pragma once

#include "assert.h"

#include <stddef.h>

namespace nstl
{
    template<typename T>
    class span
    {
    public:
        span() = default;
        span(T* data, size_t size);

        template<size_t N>
        span(T (&values)[N]);

        size_t size() const;
        bool empty() const;
        T* data() const;

        T* begin() const;
        T* end() const;

        T& operator[](size_t index) const;

        span subspan(size_t offset, size_t count = static_cast<size_t>(-1)) const;

        operator span<T const>() const;

    private:
        T* m_data = nullptr;
        size_t m_size = 0;
    };
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
nstl::span<T>::span(T* data, size_t size) : m_data(data), m_size(size)
{

}

template<typename T>
template<size_t N>
nstl::span<T>::span(T (&values)[N]) : m_data(values), m_size(N)
{

}

template<typename T>
size_t nstl::span<T>::size() const
{
    return m_size;
}

template<typename T>
bool nstl::span<T>::empty() const
{
    return m_size == 0;
}

template<typename T>
T* nstl::span<T>::data() const
{
    return m_data;
}

template<typename T>
T* nstl::span<T>::begin() const
{
    return m_data;
}

template<typename T>
T* nstl::span<T>::end() const
{
    return m_data + m_size;
}

template<typename T>
T& nstl::span<T>::operator[](size_t index) const
{
    NSTL_ASSERT(index < m_size);
    return m_data[index];
}

template<typename T>
nstl::span<T> nstl::span<T>::subspan(size_t offset, size_t count) const
{
    NSTL_ASSERT(offset <= m_size);

    if (count > m_size - offset)
        count = m_size - offset;

    NSTL_ASSERT(count <= m_size);
    NSTL_ASSERT(offset + count <= m_size);
    return { m_data + offset, count };
}

template<typename T>
nstl::span<T>::operator span<T const>() const
{
    return { m_data, m_size };
}

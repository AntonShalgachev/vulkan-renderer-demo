#pragma once

#include "span.h"

#include <stddef.h>

namespace nstl
{
    template<typename T, size_t N>
    struct array
    {
        T* data() { return m_values; }
        T const* data() const { return m_values; }
        size_t size() const { return N; }

        T* begin() { return m_values; }
        T const* begin() const { return m_values; }
        T* end() { return m_values + N }
        T const* end() const { return m_values + N }

        operator span<T>() { return { m_values, N }; }
        operator span<T const>() const { return { m_values, N }; }

        T& operator[](size_t index);
        T const& operator[](size_t index) const;

        T m_values[N];
    };

    template <class T, class... U> array(T, U...) -> array<T, 1 + sizeof...(U)>;
}

//////////////////////////////////////////////////////////////////////////

template<typename T, size_t N>
T& nstl::array<T, N>::operator[](size_t index)
{
    NSTL_ASSERT(index < N);
    return m_values[index];
}

template<typename T, size_t N>
T const& nstl::array<T, N>::operator[](size_t index) const
{
    NSTL_ASSERT(index < N);
    return m_values[index];
}

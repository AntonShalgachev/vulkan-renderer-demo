#pragma once

#include "nstl/aligned_storage.h"
#include "nstl/span.h"

#include <initializer_list>

#include <stddef.h>

namespace nstl
{
    template<typename T, size_t N>
    class static_vector
    {
    public:
        using value_type = T;

    public:
        static_vector();
//         static_vector(size_t size);
        template<typename Iterator>
        static_vector(Iterator begin, Iterator end);
        static_vector(std::initializer_list<T> list);

        ~static_vector();

        void resize(size_t new_size);
        void resize(size_t new_size, T const& value);

        void push_back(T item);
        void pop_back();

        template<typename... Args>
        T& emplace_back(Args&&... args);

        T* data();
        T const* data() const;
        size_t size() const;
        size_t capacity() const;
        bool empty() const;

        T* begin();
        T const* begin() const;
        T* end();
        T const* end() const;

        T& front();
        T const& front() const;
        T& back();
        T const& back() const;

        operator span<T>();
        operator span<T const>() const;

    private:
        nstl::aligned_storage_t<sizeof(T), alignof(T)> m_storage[N];
        size_t m_size = 0;
    };
}

//////////////////////////////////////////////////////////////////////////

template<typename T, size_t N>
nstl::static_vector<T, N>::static_vector() = default;

// template<typename T, size_t N>
// nstl::static_vector<T, N>::static_vector(size_t size)
// //     : m_buffer(size, sizeof(T), nstl::move(alloc))
// {
// //     resize(size);
// }
// 
template<typename T, size_t N>
template<typename Iterator>
nstl::static_vector<T, N>::static_vector(Iterator begin, Iterator end)
{
    m_size = end - begin;
    NSTL_ASSERT(m_size <= N);

    if constexpr (nstl::is_trivial_v<T>)
    {
        memcpy(data(), begin, m_size);
    }
    else
    {
        size_t index = 0;
        for (Iterator it = begin; it != end; it++)
            new(&m_storage[index++]) T(*it);
    }
}

template<typename T, size_t N>
nstl::static_vector<T, N>::static_vector(std::initializer_list<T> list) : static_vector(list.begin(), list.end())
{

}

template<typename T, size_t N>
nstl::static_vector<T, N>::~static_vector<T, N>()
{
    if constexpr (!nstl::is_trivial_v<T>)
    {
        for (size_t i = 0; i < m_size; i++)
            reinterpret_cast<T*>(&m_storage[i])->~T();
    }
}

template<typename T, size_t N>
void nstl::static_vector<T, N>::resize(size_t new_size)
{
    NSTL_ASSERT(new_size <= N);

    m_size = new_size;

    if constexpr (!nstl::is_trivial_v<T>)
    {
        for (size_t i = 0; i < m_size; i++)
            new (nstl::new_tag{}, &m_storage[i]) T{};
    }
}

template<typename T, size_t N>
void nstl::static_vector<T, N>::resize(size_t new_size, T const& value)
{
    m_size = new_size;

    if constexpr (!nstl::is_trivial_v<T>)
    {
        for (size_t i = 0; i < m_size; i++)
            new (nstl::new_tag{}, &m_storage[i]) T{ value };
    }
}

template<typename T, size_t N>
void nstl::static_vector<T, N>::push_back(T item)
{
    NSTL_ASSERT(m_size < N);
    new (nstl::new_tag{}, &m_storage[m_size]) T{nstl::move(item)};
    m_size++;
}

template<typename T, size_t N>
void nstl::static_vector<T, N>::pop_back()
{
    m_size--;
    if constexpr (!nstl::is_trivial_v<T>)
        reinterpret_cast<T*>(&m_storage[m_size])->~T();
}

template<typename T, size_t N>
template<typename... Args>
T& nstl::static_vector<T, N>::emplace_back(Args&&... args)
{
    NSTL_ASSERT(m_size < N);
    T* obj = new (nstl::new_tag{}, &m_storage[m_size]) T{ nstl::forward<Args>(args)... };
    m_size++;

    return *obj;
}

template<typename T, size_t N>
T* nstl::static_vector<T, N>::data()
{
    return reinterpret_cast<T*>(&m_storage[0]);
}
template<typename T, size_t N>
T const* nstl::static_vector<T, N>::data() const
{
    return reinterpret_cast<T const*>(&m_storage[0]);
}
template<typename T, size_t N>
size_t nstl::static_vector<T, N>::size() const
{
    return m_size;
}
template<typename T, size_t N>
size_t nstl::static_vector<T, N>::capacity() const
{
    return N;
}
template<typename T, size_t N>
bool nstl::static_vector<T, N>::empty() const
{
    return m_size == 0;
}

template<typename T, size_t N>
T* nstl::static_vector<T, N>::begin()
{
    return data();
}
template<typename T, size_t N>
T const* nstl::static_vector<T, N>::begin() const
{
    return data();
}
template<typename T, size_t N>
T* nstl::static_vector<T, N>::end()
{
    return data() + size();
}
template<typename T, size_t N>
T const* nstl::static_vector<T, N>::end() const
{
    return data() + size();
}

template<typename T, size_t N>
T& nstl::static_vector<T, N>::front()
{
    NSTL_ASSERT(!empty());
    return *begin();
}
template<typename T, size_t N>
T const& nstl::static_vector<T, N>::front() const
{
    NSTL_ASSERT(!empty());
    return *begin();
}
template<typename T, size_t N>
T& nstl::static_vector<T, N>::back()
{
    NSTL_ASSERT(!empty());
    return *(begin() + size() - 1);
}
template<typename T, size_t N>
T const& nstl::static_vector<T, N>::back() const
{
    NSTL_ASSERT(!empty());
    return *(begin() + size() - 1);
}

template<typename T, size_t N>
nstl::static_vector<T, N>::operator nstl::span<T>()
{
    return span<T>{ data(), size() };
}

template<typename T, size_t N>
nstl::static_vector<T, N>::operator nstl::span<T const>() const
{
    return span<T const>{ data(), size() };
}

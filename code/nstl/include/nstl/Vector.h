#pragma once

#include "Assert.h"
#include "New.h"
#include "Utility.h"

#include "Buffer.h"

#include <stddef.h>

#include <initializer_list> // TODO avoid include this header?

namespace nstl
{
    template<typename T>
    class vector
    {
    public:
        vector(size_t capacity = 0);
        vector(T const* begin, T const* end);
        vector(std::initializer_list<T> list);

        vector(vector const& rhs);
        vector(vector&& rhs);

        ~vector();

        vector& operator=(vector const& rhs);
        vector& operator=(vector&& rhs) noexcept;

        void reserve(size_t newCapacity);
        void resize(size_t newSize);

        void push_back(T item);
        void pop_back();

        void clear();

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

        T const& operator[](size_t index) const;
        T& operator[](size_t index);

        bool operator==(vector const& rhs) const;

    private:
        void grow(size_t newCapacity);

    private:
        Buffer m_buffer;
    };
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
nstl::vector<T>::vector(size_t capacity) : m_buffer(capacity, sizeof(T))
{
}

template<typename T>
nstl::vector<T>::vector(T const* begin, T const* end) : vector(end - begin)
{
    NSTL_ASSERT(capacity() >= static_cast<size_t>((end - begin)));
    NSTL_ASSERT(empty());

    for (T const* it = begin; it != end; it++)
        m_buffer.constructNext<T>(*it);
}

template<typename T>
nstl::vector<T>::vector(std::initializer_list<T> list) : vector(list.begin(), list.end())
{

}

template<typename T>
nstl::vector<T>::vector(vector const& rhs) : vector(rhs.begin(), rhs.end())
{
}

template<typename T>
nstl::vector<T>::vector(vector&& rhs) = default;

template<typename T>
nstl::vector<T>::~vector()
{
    clear();
}

template<typename T>
nstl::vector<T>& nstl::vector<T>::operator=(vector const& rhs)
{
    vector temp{ rhs };
    return (*this = nstl::move(temp));
}

template<typename T>
nstl::vector<T>& nstl::vector<T>::operator=(vector && rhs) noexcept = default;

template<typename T>
void nstl::vector<T>::reserve(size_t newCapacity)
{
    if (newCapacity > capacity())
        grow(newCapacity);
}

template<typename T>
void nstl::vector<T>::resize(size_t newSize)
{
    if (newSize > capacity())
        grow(newSize);

    NSTL_ASSERT(capacity() >= newSize);

    while (size() > newSize)
        m_buffer.destructLast<T>();
    while (size() < newSize)
        m_buffer.constructNext<T>();

    NSTL_ASSERT(size() == newSize);
}

template<typename T>
void nstl::vector<T>::push_back(T item)
{
    size_t nextSize = size() + 1;
    if (nextSize > capacity())
    {
        size_t nextCapacity = size() * 3 / 2;
        if (nextSize > nextCapacity)
            nextCapacity = nextSize;
        grow(nextCapacity);
    }

    NSTL_ASSERT(capacity() >= nextSize);

    m_buffer.constructNext<T>(nstl::move(item));
}

template<typename T>
void nstl::vector<T>::pop_back()
{
    NSTL_ASSERT(!empty());

    m_buffer.destructLast<T>();
}

template<typename T>
void nstl::vector<T>::clear()
{
    while (m_buffer.size() > 0)
        m_buffer.destructLast<T>();
}

template<typename T>
T* nstl::vector<T>::data()
{
    return begin();
}
template<typename T>
T const* nstl::vector<T>::data() const
{
    return begin();
}
template<typename T>
size_t nstl::vector<T>::size() const
{
    return m_buffer.size();
}
template<typename T>
size_t nstl::vector<T>::capacity() const
{
    return m_buffer.capacity();
}
template<typename T>
bool nstl::vector<T>::empty() const
{
    return size() == 0;
}

template<typename T>
T* nstl::vector<T>::begin()
{
    return m_buffer.get<T>(0);
}
template<typename T>
T const* nstl::vector<T>::begin() const
{
    return m_buffer.get<T>(0);
}
template<typename T>
T* nstl::vector<T>::end()
{
    return m_buffer.get<T>(size());
}
template<typename T>
T const* nstl::vector<T>::end() const
{
    return m_buffer.get<T>(size());
}

template<typename T>
T& nstl::vector<T>::front()
{
    NSTL_ASSERT(!empty());
    return (*this)[0];
}
template<typename T>
T const& nstl::vector<T>::front() const
{
    NSTL_ASSERT(!empty());
    return (*this)[0];
}
template<typename T>
T& nstl::vector<T>::back()
{
    NSTL_ASSERT(!empty());
    return (*this)[size() - 1];
}
template<typename T>
T const& nstl::vector<T>::back() const
{
    NSTL_ASSERT(!empty());
    return (*this)[size() - 1];
}

template<typename T>
T const& nstl::vector<T>::operator[](size_t index) const
{
    NSTL_ASSERT(index < m_buffer.size());
    return *m_buffer.get<T>(index);
}

template<typename T>
T& nstl::vector<T>::operator[](size_t index)
{
    NSTL_ASSERT(index < m_buffer.size());
    return *m_buffer.get<T>(index);
}

template<typename T>
bool nstl::vector<T>::operator==(vector const& rhs) const
{
    vector const& lhs = *this;
    if (lhs.size() != rhs.size())
        return false;

    size_t size = lhs.size();
    for (size_t i = 0; i < size; i++)
        if (lhs[i] != rhs[i])
            return false;

    return true;
}

template<typename T>
void nstl::vector<T>::grow(size_t newCapacity)
{
    Buffer buffer{ newCapacity, sizeof(T) };

    for (size_t i = 0; i < m_buffer.size(); i++)
        buffer.constructNext<T>(nstl::move(*m_buffer.get<T>(i)));

    NSTL_ASSERT(m_buffer.size() == buffer.size());

    while (m_buffer.size() > 0)
        m_buffer.destructLast<T>();

    m_buffer = nstl::move(buffer);

    NSTL_ASSERT(m_buffer.capacity() == newCapacity);
}

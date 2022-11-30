#pragma once

#include "assert.h"
#include "new.h"
#include "utility.h"
#include "buffer.h"
#include "span.h"
#include "type_traits.h"
#include "algorithm.h"

#include <stddef.h>

#include <initializer_list> // TODO avoid including this header?
#include <compare> // TODO avoid including this header?

// TODO distinguish between internal asserts and validations (e.g. index-out-of-bounds check should probably be active)

namespace nstl
{
    template<typename T>
    class vector
    {
    public:
        vector() = default;
        vector(size_t size);
        template<typename Iterator>
        vector(Iterator begin, Iterator end); // TODO remove?
        vector(std::initializer_list<T> list);

        vector(vector const& rhs);
        vector(vector&& rhs);

        ~vector();

        vector& operator=(vector const& rhs);
        vector& operator=(vector&& rhs) noexcept;

        void reserve(size_t newCapacity);
        void resize(size_t newSize);
        void resize(size_t newSize, T const& value);

        void push_back(T item);
        void pop_back();

        T* find(T const& value);
        T const* find(T const& value) const;

        template<typename... Args>
        T& emplace_back(Args&&... args);

        void erase(T* first, T* last);

        void erase_unsorted(T const& value);
        void erase_unsorted(T* it);

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
        auto operator<=>(vector const& rhs) const;

        operator span<T>();
        operator span<T const>() const;

    private:
        void grow(size_t newCapacity);

    private:
        Buffer m_buffer{ 0, sizeof(T) };
    };
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
nstl::vector<T>::vector(size_t size) : m_buffer(size, sizeof(T))
{
    resize(size);
}

template<typename T>
template<typename Iterator>
nstl::vector<T>::vector(Iterator begin, Iterator end) : m_buffer(end - begin, sizeof(T))
{
    NSTL_ASSERT(capacity() >= static_cast<size_t>((end - begin)));
    NSTL_ASSERT(empty());

    if constexpr (nstl::is_trivial_v<T>)
    {
        size_t size = end - begin;
        m_buffer.resize(size);
//         nstl::copy(begin, end, this->begin());
        m_buffer.copy(begin, size);
    }
    else
    {
        for (Iterator it = begin; it != end; it++)
            m_buffer.constructNext<T>(*it);
    }
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

    if constexpr (nstl::is_trivial_v<T>)
    {
        m_buffer.resize(newSize);
    }
    else
    {
        while (size() > newSize)
            m_buffer.destructLast<T>();
        while (size() < newSize)
            m_buffer.constructNext<T>();
    }

    NSTL_ASSERT(size() == newSize);
}

template<typename T>
void nstl::vector<T>::resize(size_t newSize, T const& value)
{
    if (newSize > capacity())
        grow(newSize);

    NSTL_ASSERT(capacity() >= newSize);

    if constexpr (nstl::is_trivial_v<T>)
    {
        for (size_t s = size(); s < newSize; s++)
            *(begin() + s) = value;
        m_buffer.resize(newSize);
    }
    else
    {
        while (size() > newSize)
            m_buffer.destructLast<T>();
        while (size() < newSize)
            m_buffer.constructNext<T>(value);
    }

    NSTL_ASSERT(size() == newSize);
}

template<typename T>
void nstl::vector<T>::push_back(T item)
{
    size_t nextSize = size() + 1;
    reserve(nextSize);
    NSTL_ASSERT(capacity() >= nextSize);

    if constexpr (nstl::is_trivial_v<T>)
    {
        *end() = nstl::move(item);
        m_buffer.resize(nextSize);
    }
    else
    {
        m_buffer.constructNext<T>(nstl::move(item));
    }
}

template<typename T>
void nstl::vector<T>::pop_back()
{
    NSTL_ASSERT(!empty());

    if constexpr (nstl::is_trivial_v<T>)
        resize(size() - 1);
    else
        m_buffer.destructLast<T>();
}

template<typename T>
T* nstl::vector<T>::find(T const& value)
{
    for (auto it = begin(); it != end(); it++)
    {
        if (*it == value)
            return it;
    }

    return end();
}

template<typename T>
T const* nstl::vector<T>::find(T const& value) const
{
    for (auto it = begin(); it != end(); it++)
    {
        if (*it == value)
            return it;
    }

    return end();
}

template<typename T>
template<typename... Args>
T& nstl::vector<T>::emplace_back(Args&&... args)
{
    size_t nextSize = size() + 1;
    reserve(nextSize);
    NSTL_ASSERT(capacity() >= nextSize);

    if constexpr (nstl::is_trivial_v<T>)
    {
        *end() = T{ nstl::forward<Args>(args)... };
        m_buffer.resize(nextSize);
        return back();
    }
    else
    {
        return m_buffer.constructNext<T>(nstl::forward<Args>(args)...);
    }
}

template<typename T>
void nstl::vector<T>::erase(T* first, T* last)
{
    assert(last == end()); // TODO implement erase properly

    size_t erasedCount = last - first;
    assert(size() >= erasedCount);
    resize(size() - erasedCount);
}

template<typename T>
void nstl::vector<T>::erase_unsorted(T const& value)
{
    auto it = find(value);
    return erase_unsorted(it);
}

template<typename T>
void nstl::vector<T>::erase_unsorted(T* it)
{
    if (it == end())
        return;

    nstl::exchange(*it, back());
    pop_back();
}

template<typename T>
void nstl::vector<T>::clear()
{

    if constexpr (nstl::is_trivial_v<T>)
    {
        m_buffer.resize(0);
    }
    else
    {
        while (m_buffer.size() > 0)
            m_buffer.destructLast<T>();
    }
}

template<typename T>
T* nstl::vector<T>::data()
{
    return m_buffer.get<T>(0);
}
template<typename T>
T const* nstl::vector<T>::data() const
{
    return m_buffer.get<T>(0);
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
    return m_buffer.size() == 0;
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
    return *m_buffer.get<T>(0);
}
template<typename T>
T const& nstl::vector<T>::front() const
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get<T>(0);
}
template<typename T>
T& nstl::vector<T>::back()
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get<T>(size() - 1);
}
template<typename T>
T const& nstl::vector<T>::back() const
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get<T>(size() - 1);
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
auto nstl::vector<T>::operator<=>(vector const& rhs) const
{
    // TODO use lexicographical_compare_three_way

    vector const& lhs = *this;

    T const* lhsBegin = lhs.begin();
    T const* lhsEnd = lhs.end();
    T const* rhsBegin = rhs.begin();
    T const* rhsEnd = rhs.end();

    T const* lhsIt = lhsBegin;
    T const* rhsIt = rhsBegin;

    while (lhsIt != lhsEnd)
    {
        if (rhsIt == rhsEnd)
            return std::strong_ordering::greater;
        if (auto result = (*lhsIt <=> *rhsIt); result != 0)
            return result;
        ++lhsIt;
        ++rhsIt;
    }
    return (rhsIt == rhsEnd) <=> true;
}

template<typename T>
nstl::vector<T>::operator nstl::span<T>()
{
    return span<T>{ data(), size() };
}

template<typename T>
nstl::vector<T>::operator nstl::span<T const>() const
{
    return span<T const>{ data(), size() };
}

template<typename T>
void nstl::vector<T>::grow(size_t newCapacity)
{
    Buffer buffer{ newCapacity, sizeof(T) };

    if constexpr (nstl::is_trivial_v<T>)
    {
        buffer.resize(m_buffer.size());
//         nstl::copy(m_buffer.data(), m_buffer.data() + m_buffer.size(), buffer.data());
        buffer.copy(m_buffer.data(), m_buffer.size());
    }
    else
    {
        for (size_t i = 0; i < m_buffer.size(); i++)
            buffer.constructNext<T>(nstl::move(*m_buffer.get<T>(i)));
    }

    NSTL_ASSERT(m_buffer.size() == buffer.size());

    clear();

    m_buffer = nstl::move(buffer);

    NSTL_ASSERT(m_buffer.capacity() == newCapacity);
}

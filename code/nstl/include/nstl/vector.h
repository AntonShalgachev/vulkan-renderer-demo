#pragma once

#include "assert.h"
#include "new.h"
#include "utility.h"
#include "buffer.h"
#include "span.h"
#include "type_traits.h"
#include "algorithm.h"
#include "allocator.h"

#include <stddef.h>

#include <initializer_list> // TODO avoid including this header?

// TODO distinguish between internal asserts and validations (e.g. index-out-of-bounds check should probably be active)

namespace nstl
{
    template<typename T>
    class vector
    {
    public:
        using value_type = T;

    public:
        vector(any_allocator alloc = {});
        vector(size_t size, any_allocator alloc = {});
        template<typename Iterator>
        vector(Iterator begin, Iterator end, any_allocator alloc = {}); // TODO remove?
        vector(std::initializer_list<T> list, any_allocator alloc = {});

        vector(vector const& rhs);
        vector(vector&& rhs);

        ~vector();

        vector& operator=(vector const& rhs);
        vector& operator=(vector&& rhs) noexcept;

        void reserve(size_t new_capacity);
        void resize(size_t new_size);
        void resize(size_t new_size, T const& value);

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

        operator span<T>();
        operator span<T const>() const;

    private:
        void grow(size_t new_capacity);

    private:
        buffer m_buffer;
    };
}

//////////////////////////////////////////////////////////////////////////

namespace nstl::detail
{
    inline size_t next_pow2(size_t v)
    {
        // TODO use __builtin_clzl or _BitScanForward64
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;

        return v;
    }
}

template<typename T>
nstl::vector<T>::vector(any_allocator alloc) : m_buffer(0, sizeof(T), nstl::move(alloc))
{

}

template<typename T>
nstl::vector<T>::vector(size_t size, any_allocator alloc) : m_buffer(size, sizeof(T), nstl::move(alloc))
{
    resize(size);
}

template<typename T>
template<typename Iterator>
nstl::vector<T>::vector(Iterator begin, Iterator end, any_allocator alloc) : m_buffer(static_cast<size_t>(end - begin), sizeof(T), nstl::move(alloc))
{
    NSTL_ASSERT(begin <= end);
    size_t size = static_cast<size_t>(end - begin);

    NSTL_ASSERT(capacity() >= size);
    NSTL_ASSERT(empty());

    if constexpr (nstl::is_trivial_v<T>)
    {
        m_buffer.resize(size);
        m_buffer.copy(begin, size);
    }
    else
    {
        for (Iterator it = begin; it != end; it++)
            m_buffer.constructNext<T>(*it);
    }
}

template<typename T>
nstl::vector<T>::vector(std::initializer_list<T> list, any_allocator alloc) : vector(list.begin(), list.end(), nstl::move(alloc))
{

}

template<typename T>
nstl::vector<T>::vector(vector const& rhs) : vector(rhs.begin(), rhs.end(), rhs.m_buffer.get_allocator())
{

}

template<typename T>
nstl::vector<T>::vector(vector&& rhs) = default;

template<typename T>
nstl::vector<T>::~vector<T>()
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
void nstl::vector<T>::reserve(size_t new_capacity)
{
    if (new_capacity > capacity())
        grow(new_capacity);
}

template<typename T>
void nstl::vector<T>::resize(size_t new_size)
{
    if (new_size > capacity())
        grow(new_size);

    NSTL_ASSERT(capacity() >= new_size);

    if constexpr (nstl::is_trivial_v<T>)
    {
        m_buffer.resize(new_size);
    }
    else
    {
        while (size() > new_size)
            m_buffer.destructLast<T>();
        while (size() < new_size)
            m_buffer.constructNext<T>();
    }

    NSTL_ASSERT(size() == new_size);
}

template<typename T>
void nstl::vector<T>::resize(size_t new_size, T const& value)
{
    if (new_size > capacity())
        grow(new_size);

    NSTL_ASSERT(capacity() >= new_size);

    if constexpr (nstl::is_trivial_v<T>)
    {
        for (size_t s = size(); s < new_size; s++)
            *(begin() + s) = value;
        m_buffer.resize(new_size);
    }
    else
    {
        while (size() > new_size)
            m_buffer.destructLast<T>();
        while (size() < new_size)
            m_buffer.constructNext<T>(value);
    }

    NSTL_ASSERT(size() == new_size);
}

template<typename T>
void nstl::vector<T>::push_back(T item)
{
    size_t next_size = size() + 1;
    reserve(next_size);
    NSTL_ASSERT(capacity() >= next_size);

    if constexpr (nstl::is_trivial_v<T>)
    {
        *end() = nstl::move(item);
        m_buffer.resize(next_size);
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
    size_t next_size = size() + 1;
    reserve(next_size);
    NSTL_ASSERT(capacity() >= next_size);

    if constexpr (nstl::is_trivial_v<T>)
    {
        *end() = T{ nstl::forward<Args>(args)... };
        m_buffer.resize(next_size);
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
    NSTL_ASSERT(last == end()); // TODO implement erase properly

    NSTL_ASSERT(first <= last);
    size_t erasedCount = static_cast<size_t>(last - first);
    NSTL_ASSERT(size() >= erasedCount);
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
bool nstl::vector<T>::operator==(vector const& rhs) const
{
    if (size() != rhs.size())
        return false;

    for (size_t i = 0; i < size(); i++)
        if ((*this)[i] != rhs[i])
            return false;

    return true;
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
void nstl::vector<T>::grow(size_t new_capacity)
{
    new_capacity = detail::next_pow2(new_capacity);

    // TODO don't copy allocator
    buffer newBuffer{ new_capacity, sizeof(T), m_buffer.get_allocator() };

    if constexpr (nstl::is_trivial_v<T>)
    {
        newBuffer.resize(m_buffer.size());
        newBuffer.copy(m_buffer.data(), m_buffer.size());
    }
    else
    {
        for (size_t i = 0; i < m_buffer.size(); i++)
            newBuffer.constructNext<T>(nstl::move(*m_buffer.get<T>(i)));
    }

    NSTL_ASSERT(m_buffer.size() == newBuffer.size());

    clear();

    m_buffer = nstl::move(newBuffer);

    NSTL_ASSERT(m_buffer.capacity() == new_capacity);
}

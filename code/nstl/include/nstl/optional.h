#pragma once

#include "assert.h"
#include "new.h"
#include "utility.h"

namespace nstl
{
    template<typename T>
    class optional
    {
    public:
        optional();
        optional(T const& value);
        optional(T&& value);

        optional(optional const& rhs);
        optional(optional&& rhs);

        ~optional();

        optional& operator=(optional const& rhs);
        optional& operator=(optional&& rhs);

        bool has_value() const;
        explicit operator bool() const;

        T const& operator*() const;
        T& operator*();

        T const* operator->() const;
        T* operator->();

        bool operator==(optional const& rhs) const;
        bool operator!=(optional const& rhs) const;

        // TODO make if out-of-line?
        template<typename T2>
        friend bool operator==(optional const& lhs, T2 const& rhs)
        {
            return lhs.m_hasValue && lhs.m_value == rhs;
        }
        template<typename T2>
        friend bool operator!=(optional const& lhs, T2 const& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        void construct(T const& value);
        void construct(T&& value);
        void destroy();

    private:
        struct Dummy
        {
        };

        bool m_hasValue = false;
        union
        {
            Dummy m_empty;
            T m_value;
        };
    };
}

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702) // unreachable code
#endif
template<typename T>
nstl::optional<T>::optional() : m_hasValue(false), m_empty({})
{
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<typename T>
nstl::optional<T>::optional(T const& value) : optional()
{
    construct(value);
}

template<typename T>
nstl::optional<T>::optional(T&& value) : optional()
{
    construct(nstl::move(value));
}

template<typename T>
nstl::optional<T>::optional(optional const& rhs) : optional()
{
    if (rhs.m_hasValue)
        construct(rhs.m_value);
}

template<typename T>
nstl::optional<T>::optional(optional&& rhs) : optional()
{
    if (rhs.m_hasValue)
        construct(nstl::move(rhs.m_value));
}

template<typename T>
nstl::optional<T>::~optional()
{
    destroy();
}

template<typename T>
nstl::optional<T>& nstl::optional<T>::operator=(optional const& rhs)
{
    if (m_hasValue && rhs.m_hasValue)
        m_value = rhs.m_value;
    else if (m_hasValue && !rhs.m_hasValue)
        destroy();
    else if (!m_hasValue && rhs.m_hasValue)
        construct(rhs.m_value);

    return *this;
}

template<typename T>
nstl::optional<T>& nstl::optional<T>::operator=(optional&& rhs)
{
    if (m_hasValue && rhs.m_hasValue)
        m_value = nstl::move(rhs.m_value);
    else if (m_hasValue && !rhs.m_hasValue)
        destroy();
    else if (!m_hasValue && rhs.m_hasValue)
        construct(nstl::move(rhs.m_value));

    return *this;
}

template<typename T>
bool nstl::optional<T>::has_value() const
{
    return m_hasValue;
}

template<typename T>
nstl::optional<T>::operator bool() const
{
    return m_hasValue;
}

template<typename T>
T const& nstl::optional<T>::operator*() const
{
    NSTL_ASSERT(m_hasValue);
    return m_value;
}

template<typename T>
T& nstl::optional<T>::operator*()
{
    NSTL_ASSERT(m_hasValue);
    return m_value;
}

template<typename T>
T const* nstl::optional<T>::operator->() const
{
    return &m_value;
}

template<typename T>
T* nstl::optional<T>::operator->()
{
    return &m_value;
}

template<typename T>
bool nstl::optional<T>::operator==(optional const& rhs) const
{
    if (m_hasValue != rhs.m_hasValue)
        return false;

    if (m_hasValue && rhs.m_hasValue)
        return m_value == rhs.m_value;

    return true;
}

template<typename T>
bool nstl::optional<T>::operator!=(optional const& rhs) const
{
    return !(*this == rhs);
}

template<typename T>
void nstl::optional<T>::construct(T const& value)
{
    new (nstl::NewTag{}, &m_value) T(value);
    m_hasValue = true;
}

template<typename T>
void nstl::optional<T>::construct(T&& value)
{
    new (nstl::NewTag{}, &m_value) T(nstl::move(value));
    m_hasValue = true;
}

template<typename T>
void nstl::optional<T>::destroy()
{
    if (m_hasValue)
        m_value.~T();

    m_empty = {};
    m_hasValue = false;
}

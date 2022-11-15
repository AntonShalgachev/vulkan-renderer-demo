#pragma once

#include "Assert.h"
#include "New.h"
#include "Utility.h"

namespace nstl
{
    template<typename T>
    class Optional
    {
    public:
        Optional();
        Optional(T const& value);
        Optional(T&& value);

        Optional(Optional const& rhs);
        Optional(Optional&& rhs);

        ~Optional();

        Optional& operator=(Optional const& rhs);
        Optional& operator=(Optional&& rhs);

        bool hasValue() const;
        operator bool() const;

        T const& operator*() const;
        T& operator*();

        T const* operator->() const;
        T* operator->();

        bool operator==(Optional const& rhs) const;
        bool operator!=(Optional const& rhs) const;

        template<typename T2>
        bool operator==(T2 const& rhs) const;
        template<typename T2>
        bool operator!=(T2 const& rhs) const;

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
nstl::Optional<T>::Optional() : m_hasValue(false), m_empty({})
{
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<typename T>
nstl::Optional<T>::Optional(T const& value) : Optional()
{
    construct(value);
}

template<typename T>
nstl::Optional<T>::Optional(T&& value) : Optional()
{
    construct(nstl::move(value));
}

template<typename T>
nstl::Optional<T>::Optional(Optional const& rhs) : Optional()
{
    if (rhs.m_hasValue)
        construct(rhs.m_value);
}

template<typename T>
nstl::Optional<T>::Optional(Optional&& rhs) : Optional()
{
    if (rhs.m_hasValue)
        construct(nstl::move(rhs.m_value));
}

template<typename T>
nstl::Optional<T>::~Optional()
{
    destroy();
}

template<typename T>
nstl::Optional<T>& nstl::Optional<T>::operator=(Optional const& rhs)
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
nstl::Optional<T>& nstl::Optional<T>::operator=(Optional&& rhs)
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
bool nstl::Optional<T>::hasValue() const
{
    return m_hasValue;
}

template<typename T>
nstl::Optional<T>::operator bool() const
{
    return m_hasValue;
}

template<typename T>
T const& nstl::Optional<T>::operator*() const
{
    NSTL_ASSERT(m_hasValue);
    return m_value;
}

template<typename T>
T& nstl::Optional<T>::operator*()
{
    NSTL_ASSERT(m_hasValue);
    return m_value;
}

template<typename T>
T const* nstl::Optional<T>::operator->() const
{
    return &m_value;
}

template<typename T>
T* nstl::Optional<T>::operator->()
{
    return &m_value;
}

template<typename T>
bool nstl::Optional<T>::operator==(Optional const& rhs) const
{
    if (m_hasValue != rhs.m_hasValue)
        return false;

    if (m_hasValue && rhs.m_hasValue)
        return m_value == rhs.m_value;

    return true;
}

template<typename T>
bool nstl::Optional<T>::operator!=(Optional const& rhs) const
{
    return !(*this == rhs);
}

template<typename T>
template<typename T2>
bool nstl::Optional<T>::operator==(T2 const& rhs) const
{
    return m_hasValue && m_value == rhs;
}

template<typename T>
template<typename T2>
bool nstl::Optional<T>::operator!=(T2 const& rhs) const
{
    return !(*this == rhs);
}

template<typename T>
void nstl::Optional<T>::construct(T const& value)
{
    new (nstl::NewTag{}, &m_value) T(value);
    m_hasValue = true;
}

template<typename T>
void nstl::Optional<T>::construct(T&& value)
{
    new (nstl::NewTag{}, &m_value) T(nstl::move(value));
    m_hasValue = true;
}

template<typename T>
void nstl::Optional<T>::destroy()
{
    if (m_hasValue)
        m_value.~T();

    m_empty = {};
    m_hasValue = false;
}

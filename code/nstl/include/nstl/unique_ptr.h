#pragma once

#include "nstl/assert.h"
#include "nstl/utility.h"

#include <stddef.h>

namespace nstl
{
    template<typename T>
    class unique_ptr
    {
    public:
        unique_ptr() = default;
        explicit unique_ptr(T* ptr) : m_ptr(ptr) {}
        unique_ptr(nullptr_t) {}
        unique_ptr(unique_ptr&& rhs)
        {
            nstl::exchange(m_ptr, rhs.m_ptr);
        }
        ~unique_ptr()
        {
            if (m_ptr)
                delete m_ptr;
        }

        unique_ptr& operator=(unique_ptr&& rhs)
        {
            nstl::exchange(m_ptr, rhs.m_ptr);
            return *this;
        }

        T* get() const
        {
            return m_ptr;
        }

        explicit operator bool() const
        {
            return m_ptr != nullptr;
        }

        T& operator*() const
        {
            NSTL_ASSERT(m_ptr);
            return *m_ptr;
        }

        T* operator->() const
        {
            NSTL_ASSERT(m_ptr);
            return m_ptr;
        }

        template<typename T2>
        operator unique_ptr<T2>()&&
        {
            unique_ptr<T2> result{ m_ptr };
            m_ptr = nullptr;
            return result;
        }

    private:
        T* m_ptr = nullptr;
    };

    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        return unique_ptr<T>(new T(nstl::forward<Args>(args)...));
    }
}

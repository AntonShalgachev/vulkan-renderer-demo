#pragma once

#include "assert.h"
#include "utility.h"

namespace nstl
{
    template<typename T>
    class UniquePtr
    {
    public:
        explicit UniquePtr(T* ptr = nullptr) : m_ptr(ptr) {}
        UniquePtr(UniquePtr&& rhs)
        {
            nstl::exchange(m_ptr, rhs.m_ptr);
        }
        ~UniquePtr()
        {
            if (m_ptr)
                delete m_ptr;
        }

        UniquePtr& operator=(UniquePtr&& rhs)
        {
            nstl::exchange(m_ptr, rhs.m_ptr);
            return *this;
        }

        T* get()
        {
            return m_ptr;
        }
        T const* get() const
        {
            return m_ptr;
        }

        explicit operator bool() const
        {
            return m_ptr != nullptr;
        }

        T const& operator*() const
        {
            NSTL_ASSERT(m_ptr);
            return *m_ptr;
        }
        T& operator*()
        {
            NSTL_ASSERT(m_ptr);
            return *m_ptr;
        }

        T const* operator->() const
        {
            NSTL_ASSERT(m_ptr);
            return m_ptr;
        }
        T* operator->()
        {
            NSTL_ASSERT(m_ptr);
            return m_ptr;
        }

        template<typename T2>
        operator UniquePtr<T2>()&&
        {
            UniquePtr<T2> result{ m_ptr };
            m_ptr = nullptr;
            return result;
        }

    private:
        T* m_ptr = nullptr;
    };

    template<typename T, typename... Args>
    UniquePtr<T> makeUnique(Args&&... args)
    {
        return UniquePtr<T>(new T(nstl::forward<Args>(args)...));
    }
}

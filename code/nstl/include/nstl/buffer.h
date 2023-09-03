#pragma once

#include "assert.h"
#include "new.h"
#include "utility.h"
#include "allocator.h"

#include <stddef.h>

namespace nstl
{
    class buffer
    {
    public:
        buffer(size_t capacity, size_t chunkSize, any_allocator alloc);
        buffer(buffer const& rhs);
        buffer(buffer&& rhs);
        ~buffer();

        buffer& operator=(buffer const& rhs);
        buffer& operator=(buffer&& rhs);

        char* data();
        char const* data() const;
        size_t capacity() const;
        size_t capacityBytes() const;
        size_t size() const;

        void resize(size_t newSize);

        char* get(size_t index);
        char const* get(size_t index) const;

        void copy(void const* ptr, size_t size);

        template<typename T, typename... Args>
        T& constructNext(Args&&... args)
        {
            NSTL_ASSERT(m_ptr);
            NSTL_ASSERT(m_size < m_capacity);
            NSTL_ASSERT(m_chunkSize == sizeof(T));

            T* obj = new (new_tag{}, m_ptr + m_size * sizeof(T)) T(nstl::forward<Args>(args)...);
            m_size++;

#if NSTL_CONFIG_ENABLE_ASSERTS
            m_constructedObjectsCount++;
#endif
            NSTL_ASSERT(obj);
            return *obj;
        }

        template<typename T>
        void destructLast()
        {
            NSTL_ASSERT(m_size > 0);
            NSTL_ASSERT(m_chunkSize == sizeof(T));

            get<T>(m_size - 1)->~T();
            m_size--;

#if NSTL_CONFIG_ENABLE_ASSERTS
            NSTL_ASSERT(m_constructedObjectsCount > 0);
            m_constructedObjectsCount--;
#endif
        }

        template<typename T>
        T* get(size_t index)
        {
            return reinterpret_cast<T*>(get(index * sizeof(T)));
        }

        template<typename T>
        T const* get(size_t index) const
        {
            return reinterpret_cast<T const*>(get(index * sizeof(T)));
        }

        any_allocator const& get_allocator() const;

    private:
        void swap(buffer& rhs) noexcept;

    private:
        any_allocator m_allocator;

        char* m_ptr = nullptr;
        size_t m_capacity = 0;
        size_t m_chunkSize = 0;

        size_t m_size = 0;

#if NSTL_CONFIG_ENABLE_ASSERTS
        size_t m_constructedObjectsCount = 0;
#endif
    };
}

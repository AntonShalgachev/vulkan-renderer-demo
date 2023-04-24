#pragma once

#include <vulkan/vulkan.h>

namespace vko
{
    // Its main purpose is to make it easier to implement move constructors of vulkan objects
    template<typename T>
    class UniqueHandle
    {
    public:
        UniqueHandle() = default;
        explicit UniqueHandle(nullptr_t) : UniqueHandle() {}
        UniqueHandle(T const& handle) : m_handle(handle) {}

        UniqueHandle(UniqueHandle const&) = delete;
        UniqueHandle(UniqueHandle&& rhs) : m_handle(rhs.m_handle)
        {
            rhs.m_handle = VK_NULL_HANDLE;
        }
        UniqueHandle& operator=(UniqueHandle const&) = delete;
        UniqueHandle& operator=(UniqueHandle&& rhs)
        {
            if (this != &rhs)
            {
                m_handle = rhs.m_handle;
                rhs.m_handle = VK_NULL_HANDLE;
            }

            return *this;
        }

        UniqueHandle& operator=(nullptr_t)
        {
            m_handle = VK_NULL_HANDLE;
            return *this;
        }

        T const& get() const { return m_handle; }
        T& get() { return m_handle; }

        operator bool() const
        {
            return m_handle != VK_NULL_HANDLE;
        }

        operator T() const
        {
            return m_handle;
        }

        ~UniqueHandle() = default;

    private:
        T m_handle = VK_NULL_HANDLE;
    };
}

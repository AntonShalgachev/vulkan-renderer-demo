#pragma once

#include <vulkan/vulkan.h>

#include "Utils.h" // TODO remove

namespace vkr
{
    // Its main purpose is to make it easier to implement move constructors of vulkan objects
    template<typename T>
    class UniqueHandle
    {
    public:
        UniqueHandle() = default;
        explicit UniqueHandle(std::nullptr_t) : UniqueHandle() {}
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

        UniqueHandle& operator=(std::nullptr_t)
        {
            m_handle = VK_NULL_HANDLE;
            return *this;
        }

        T const& get() const { return m_handle; }
        T& get() { return m_handle; }

        operator T() const
        {
            return m_handle;
        }

        ~UniqueHandle()
        {
            // Nothing to do; handle destruction is very custom and handled in the object itself
        }

    private:
        T m_handle = VK_NULL_HANDLE;
    };
}

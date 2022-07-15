#pragma once

#include <vulkan/vulkan.h>

// TODO move somewhere
#include <string>
#include <stdexcept>
#define VKR_ASSERT(cmd) do { if (auto res = (cmd); res != VK_SUCCESS) throw std::runtime_error(std::to_string(res)); } while(0)

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

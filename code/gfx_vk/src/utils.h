#pragma once

#include "common/fmt.h"

#include <vulkan/vulkan.h>

#include <assert.h>

namespace gfx_vk
{
    template<typename T>
    class unique_handle
    {
    public:
        unique_handle() = default;
        explicit unique_handle(nullptr_t) : unique_handle() {}
        unique_handle(T const& handle) : m_handle(handle) {}

        unique_handle(unique_handle const&) = delete;
        unique_handle(unique_handle&& rhs) : m_handle(rhs.m_handle)
        {
            rhs.m_handle = VK_NULL_HANDLE;
        }
        unique_handle& operator=(unique_handle const&) = delete;
        unique_handle& operator=(unique_handle&& rhs)
        {
            if (this != &rhs)
            {
                m_handle = rhs.m_handle;
                rhs.m_handle = VK_NULL_HANDLE;
            }

            return *this;
        }

        unique_handle& operator=(nullptr_t)
        {
            m_handle = VK_NULL_HANDLE;
            return *this;
        }

        T const& get() const { return m_handle; }
        T& get() { return m_handle; }

        explicit operator bool() const
        {
            return m_handle != VK_NULL_HANDLE;
        }

        operator T() const
        {
            return m_handle;
        }

        ~unique_handle() = default;

    private:
        T m_handle = VK_NULL_HANDLE;
    };

    inline void do_assert(VkResult result, char const*)
    {
        assert(result == VK_SUCCESS);
    }
}

#ifdef NDEBUG
#define GFX_VK_VERIFY(cmd) (cmd)
#else
#define GFX_VK_VERIFY(cmd) gfx_vk::do_assert((cmd), #cmd)
#endif

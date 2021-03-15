#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    class Instance;
    class Window;

    class Surface
    {
    public:
        explicit Surface(Instance const& instance, Window const& window);
        ~Surface();

        Surface(Surface const&) = default;
        Surface(Surface&&) = default;
        Surface& operator=(Surface const&) = default;
        Surface& operator=(Surface&&) = default;

        VkSurfaceKHR getHandle() const { return m_handle; }

        int getWidth() const;
        int getHeight() const;

    private:
        UniqueHandle<VkSurfaceKHR> m_handle;

        Instance const& m_instance;
        Window const& m_window;
    };
}

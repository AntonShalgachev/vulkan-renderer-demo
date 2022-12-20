#pragma once

#include "renderer/wrapper/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Instance;
    class Window;

    class Surface
    {
    public:
        explicit Surface(VkSurfaceKHR handle, Instance const& instance, vko::Window const& window);
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
        vko::Window const& m_window;
    };
}

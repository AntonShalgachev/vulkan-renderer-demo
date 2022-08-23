#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

// TODO remove
namespace vkr
{
    class Window;
}

namespace vko
{
    class Instance;

    class Surface
    {
    public:
        explicit Surface(Instance const& instance, vkr::Window const& window);
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
        vkr::Window const& m_window;
    };
}

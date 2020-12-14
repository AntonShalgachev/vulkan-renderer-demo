#pragma once

#include <vulkan/vulkan.h>

namespace vkr
{
    class Instance;
    class Window;

    class VulkanSurfaceCreator
    {
    public:
        static VkSurfaceKHR createVulkanSurface(Instance const& instance, Window const& window);
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    class Instance;
    class Window;

    class VulkanSurfaceCreator
    {
    public:
        static UniqueHandle<VkSurfaceKHR> createVulkanSurface(Instance const& instance, Window const& window);
    };
}

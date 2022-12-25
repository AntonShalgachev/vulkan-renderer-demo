#pragma once

#include "nstl/vector.h"

#include <vulkan/vulkan.h> // TODO remove from the header

namespace vko
{
    class PhysicalDevice;
    class Surface;
    class QueueFamily;

    struct PhysicalDeviceSurfaceParameters
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        nstl::vector<VkSurfaceFormatKHR> formats;
        nstl::vector<VkPresentModeKHR> presentModes;
        vko::QueueFamily const* graphicsQueueFamily = nullptr;
        vko::QueueFamily const* presentQueueFamily = nullptr;
    };

    PhysicalDeviceSurfaceParameters queryPhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface);
}

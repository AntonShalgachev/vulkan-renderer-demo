#pragma once

#include "nstl/optional.h"
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
        nstl::optional<uint32_t> graphicsQueueFamily;
        nstl::optional<uint32_t> presentQueueFamily;
    };

    PhysicalDeviceSurfaceParameters queryPhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, VkSurfaceKHR surface);
}

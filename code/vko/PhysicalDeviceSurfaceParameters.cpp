#include "PhysicalDeviceSurfaceParameters.h"

#include "Assert.h"
#include "PhysicalDevice.h"
#include "Surface.h"

namespace
{
    VkSurfaceCapabilitiesKHR queryCapabilities(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        VkSurfaceCapabilitiesKHR result{};
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.getHandle(), surface.getHandle(), &result));
        return result;
    }

    nstl::vector<VkSurfaceFormatKHR> queryFormats(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        if (count <= 0)
            return {};

        nstl::vector<VkSurfaceFormatKHR> result;
        result.resize(count);

        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));

        return result;
    }

    nstl::vector<VkPresentModeKHR> queryPresentModes(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        if (count <= 0)
            return {};

        nstl::vector<VkPresentModeKHR> result;
        result.resize(count);

        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));

        return result;
    }

    bool queryPresentationSupport(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface, vko::QueueFamily const& queueFamily)
    {
        VkBool32 result = false;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), queueFamily.getIndex(), surface.getHandle(), &result));
        return result;
    }
}

vko::PhysicalDeviceSurfaceParameters vko::queryPhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface)
{
    PhysicalDeviceSurfaceParameters params;
    params.capabilities = queryCapabilities(physicalDevice, surface);
    params.formats = queryFormats(physicalDevice, surface);
    params.presentModes = queryPresentModes(physicalDevice, surface);

    for (QueueFamily const& queueFamily : physicalDevice.getQueueFamilies())
    {
        if (queueFamily.getProperties().queueFlags & VK_QUEUE_GRAPHICS_BIT)
            params.graphicsQueueFamily = &queueFamily;

        if (queryPresentationSupport(physicalDevice, surface, queueFamily))
            params.presentQueueFamily = &queueFamily;

        if (params.graphicsQueueFamily && params.presentQueueFamily)
            break;
    }

    return params;
}


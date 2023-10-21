#include "PhysicalDeviceSurfaceParameters.h"

#include "Assert.h"
#include "PhysicalDevice.h"
#include "Surface.h"

vko::PhysicalDeviceSurfaceParameters vko::queryPhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, VkSurfaceKHR surface)
{
    VkPhysicalDevice handle = physicalDevice.getHandle();
    nstl::span<VkQueueFamilyProperties const> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    PhysicalDeviceSurfaceParameters params{};
    VKO_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &params.capabilities));

    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, nullptr));
        params.formats.resize(count);
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, params.formats.data()));
    }

    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count, nullptr));
        params.presentModes.resize(count);
        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count, params.presentModes.data()));
    }

    for (uint32_t familyIndex = 0; familyIndex < queueFamilyProperties.size(); familyIndex++)
//     for (QueueFamily const& queueFamily : queueFamilies)
    {
        VkBool32 hasPresentationSupport = false;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(handle, familyIndex, surface, &hasPresentationSupport));

        if (queueFamilyProperties[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            params.graphicsQueueFamily = familyIndex;

        if (hasPresentationSupport)
            params.presentQueueFamily = familyIndex;

        if (params.graphicsQueueFamily && params.presentQueueFamily)
            break;
    }

    return params;
}


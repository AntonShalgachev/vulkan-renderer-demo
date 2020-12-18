#include "QueueFamilyIndices.h"

#include <optional>
#include <vector>

#include "PhysicalDevice.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamily.h"

vkr::QueueFamilyIndices::QueueFamilyIndices(vkr::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters)
{
    for (QueueFamily const& queueFamily : physicalDevice.getQueueFamilies())
    {
        if (queueFamily.getProperties().queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_graphicsQueueFamily = &queueFamily;

        if (physicalDeviceSurfaceParameters.isPresentationSupported(queueFamily))
            m_presentQueueFamily = &queueFamily;

        if (isValid())
            break;
    }
}

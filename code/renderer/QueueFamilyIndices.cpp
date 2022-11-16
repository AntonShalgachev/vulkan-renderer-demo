#include "QueueFamilyIndices.h"

#include "wrapper/PhysicalDevice.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "wrapper/QueueFamily.h"

vkr::QueueFamilyIndices::QueueFamilyIndices(vko::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters)
{
    for (vko::QueueFamily const& queueFamily : physicalDevice.getQueueFamilies())
    {
        if (queueFamily.getProperties().queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_graphicsQueueFamily = &queueFamily;

        if (physicalDeviceSurfaceParameters.isPresentationSupported(queueFamily))
            m_presentQueueFamily = &queueFamily;

        if (isValid())
            break;
    }
}

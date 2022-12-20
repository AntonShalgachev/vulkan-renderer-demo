#include "QueueFamilyIndices.h"

#include "renderer/wrapper/PhysicalDevice.h"
#include "renderer/wrapper/QueueFamily.h"

#include "renderer/PhysicalDeviceSurfaceParameters.h"

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

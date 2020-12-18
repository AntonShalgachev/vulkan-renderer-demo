#include "QueueFamilyIndices.h"

#include <optional>
#include <vector>

#include "PhysicalDevice.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamily.h"

namespace
{
    struct OptionalIndices
    {
        std::optional<uint32_t> graphicsFamilyIndex;
        std::optional<uint32_t> presentFamilyIndex;

        bool isValid()
        {
            return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
        }
    };
}

vkr::QueueFamilyIndices::QueueFamilyIndices(vkr::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters)
{
    OptionalIndices indices;

    for (QueueFamily const& queueFamily : physicalDevice.getQueueFamilies())
    {
        uint32_t index = queueFamily.getIndex();

        if (queueFamily.getProperties().queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamilyIndex = index;

        if (physicalDeviceSurfaceParameters.isPresentationSupported(queueFamily))
            indices.presentFamilyIndex = index;

        if (indices.isValid())
            break;
    }

    m_isValid = indices.isValid();

    if (indices.graphicsFamilyIndex.has_value())
        m_graphicsQueueIndex = indices.graphicsFamilyIndex.value();

    if (indices.presentFamilyIndex.has_value())
        m_presentQueueIndex = indices.presentFamilyIndex.value();
}

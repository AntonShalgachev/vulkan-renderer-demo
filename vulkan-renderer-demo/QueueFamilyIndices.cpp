#include "QueueFamilyIndices.h"

#include <optional>
#include <vector>

#include "PhysicalDevice.h"
#include "PhysicalDeviceSurfaceParameters.h"

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

    std::vector<VkQueueFamilyProperties> const& queueFamilies = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        auto const& queueFamily = queueFamilies[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamilyIndex = i;

        if (physicalDeviceSurfaceParameters.isPresentationSupported(i))
            indices.presentFamilyIndex = i;

        if (indices.isValid())
            break;
    }

    m_isValid = indices.isValid();

    if (indices.graphicsFamilyIndex.has_value())
        m_graphicsQueueIndex = indices.graphicsFamilyIndex.value();

    if (indices.presentFamilyIndex.has_value())
        m_presentQueueIndex = indices.presentFamilyIndex.value();
}

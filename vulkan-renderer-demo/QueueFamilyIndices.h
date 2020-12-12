#pragma once

#include <vulkan\vulkan.h>

namespace vkr
{
    class PhysicalDevice;
    class PhysicalDeviceSurfaceParameters;

    class QueueFamilyIndices
    {
    public:
        QueueFamilyIndices(vkr::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters);

        bool isValid() const { return m_isValid; }

        uint32_t getGraphicsIndex() const { return m_graphicsQueueIndex; }
        uint32_t getPresentIndex() const { return m_presentQueueIndex; }

    private:
        bool m_isValid = false;

        uint32_t m_graphicsQueueIndex = 0;
        uint32_t m_presentQueueIndex = 0;
    };
}

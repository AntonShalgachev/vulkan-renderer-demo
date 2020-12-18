#pragma once

#include <vulkan\vulkan.h>

namespace vkr
{
    class PhysicalDevice;
    class PhysicalDeviceSurfaceParameters;
    class QueueFamily;

    class QueueFamilyIndices
    {
    public:
        QueueFamilyIndices(vkr::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters);

        bool isValid() const { return m_graphicsQueueFamily && m_presentQueueFamily; }

        QueueFamily const& getGraphicsQueueFamily() const { return *m_graphicsQueueFamily; }
        QueueFamily const& getPresentQueueFamily() const { return *m_presentQueueFamily; }

    private:
        QueueFamily const* m_graphicsQueueFamily = nullptr;
        QueueFamily const* m_presentQueueFamily = nullptr;
    };
}

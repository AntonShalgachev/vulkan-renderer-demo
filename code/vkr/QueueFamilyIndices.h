#pragma once

#include <vulkan\vulkan.h>

namespace vko
{
    class PhysicalDevice;
    class QueueFamily;
}

namespace vkr
{
    class PhysicalDeviceSurfaceParameters;

    class QueueFamilyIndices
    {
    public:
        QueueFamilyIndices(vko::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters);

        bool isValid() const { return m_graphicsQueueFamily && m_presentQueueFamily; }

        vko::QueueFamily const& getGraphicsQueueFamily() const { return *m_graphicsQueueFamily; }
        vko::QueueFamily const& getPresentQueueFamily() const { return *m_presentQueueFamily; }

    private:
        vko::QueueFamily const* m_graphicsQueueFamily = nullptr;
        vko::QueueFamily const* m_presentQueueFamily = nullptr;
    };
}

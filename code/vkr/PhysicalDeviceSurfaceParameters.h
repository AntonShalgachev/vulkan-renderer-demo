#pragma once

#include "nstl/vector.h"
#include "nstl/span.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class PhysicalDevice;
    class Surface;
    class QueueFamily;
}

namespace vkr
{
    class PhysicalDeviceSurfaceParameters
    {
    public:
    	PhysicalDeviceSurfaceParameters(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface);
        ~PhysicalDeviceSurfaceParameters();

        PhysicalDeviceSurfaceParameters(PhysicalDeviceSurfaceParameters const&) = delete;
        PhysicalDeviceSurfaceParameters(PhysicalDeviceSurfaceParameters&&);
        PhysicalDeviceSurfaceParameters& operator=(PhysicalDeviceSurfaceParameters const&) = delete;
        PhysicalDeviceSurfaceParameters& operator=(PhysicalDeviceSurfaceParameters&&) = delete;

        VkSurfaceCapabilitiesKHR const& getCapabilities() const { return m_capabilities; }
        nstl::span<VkSurfaceFormatKHR const> getFormats() const { return m_formats; }
        nstl::span<VkPresentModeKHR const> getPresentModes() const { return m_presentModes; }
        bool isPresentationSupported(vko::QueueFamily const& queueFamily) const;

        void onSurfaceChanged();

        bool areQueueFamiliesComplete() const { return m_graphicsQueueFamily && m_presentQueueFamily; }
        vko::QueueFamily const& getGraphicsQueueFamily() const { return *m_graphicsQueueFamily; }
        vko::QueueFamily const& getPresentQueueFamily() const { return *m_presentQueueFamily; }

    private:
        vko::PhysicalDevice const& m_physicalDevice;
        vko::Surface const& m_surface;

        VkSurfaceCapabilitiesKHR m_capabilities;
        nstl::vector<VkSurfaceFormatKHR> m_formats;
        nstl::vector<VkPresentModeKHR> m_presentModes;
        nstl::vector<bool> m_queuePresentationSupport;

        vko::QueueFamily const* m_graphicsQueueFamily = nullptr;
        vko::QueueFamily const* m_presentQueueFamily = nullptr;
    };
}

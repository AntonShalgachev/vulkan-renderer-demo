#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice;
    class Surface;

    class PhysicalDeviceSurfaceParameters
    {
    public:
    	PhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface);

        VkSurfaceCapabilitiesKHR const& getCapabilities() const { return m_capabilities; }
        std::vector<VkSurfaceFormatKHR> const& getFormats() const { return m_formats; }
        std::vector<VkPresentModeKHR> getPresentModes() const { return m_presentModes; }
        bool isPresentationSupported(std::size_t queueIndex) const { return m_queuePresentationSupport[queueIndex]; }

    private:
        VkSurfaceCapabilitiesKHR m_capabilities;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_presentModes;
        std::vector<bool> m_queuePresentationSupport;
    };
}

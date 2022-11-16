#pragma once

#include "QueueFamilyIndices.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class PhysicalDevice;
    class Surface;
    class QueueFamily;
}

namespace vkr
{
    class QueueFamilyIndices;

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
        nstl::vector<VkSurfaceFormatKHR> const& getFormats() const { return m_formats; }
        nstl::vector<VkPresentModeKHR> const& getPresentModes() const { return m_presentModes; }
        bool isPresentationSupported(vko::QueueFamily const& queueFamily) const;

        void onSurfaceChanged();

        QueueFamilyIndices const& getQueueFamilyIndices() const { return m_queueFamilyIndices; };

    private:
        vko::PhysicalDevice const& m_physicalDevice;
        vko::Surface const& m_surface;

        VkSurfaceCapabilitiesKHR m_capabilities;
        nstl::vector<VkSurfaceFormatKHR> m_formats;
        nstl::vector<VkPresentModeKHR> m_presentModes;
        nstl::vector<bool> m_queuePresentationSupport;

        QueueFamilyIndices m_queueFamilyIndices;
    };
}

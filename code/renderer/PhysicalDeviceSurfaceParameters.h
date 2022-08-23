#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

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
        std::vector<VkSurfaceFormatKHR> const& getFormats() const { return m_formats; }
        std::vector<VkPresentModeKHR> getPresentModes() const { return m_presentModes; }
        bool isPresentationSupported(vko::QueueFamily const& queueFamily) const;

        void onSurfaceChanged();

        QueueFamilyIndices const& getQueueFamilyIndices() const { return *m_queueFamilyIndices; };

    private:
        void queryCapabilities();
        void queryFormats();
        void queryPresentModes();
        void queryPresentationSupport();

    private:
        VkSurfaceCapabilitiesKHR m_capabilities;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_presentModes;
        std::vector<bool> m_queuePresentationSupport;

        std::unique_ptr<QueueFamilyIndices> m_queueFamilyIndices;

        vko::PhysicalDevice const& m_physicalDevice;
        vko::Surface const& m_surface;
    };
}

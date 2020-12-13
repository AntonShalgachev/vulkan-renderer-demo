#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice;
    class Surface;
    class QueueFamilyIndices;

    class PhysicalDeviceSurfaceParameters
    {
    public:
    	PhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface);

        VkSurfaceCapabilitiesKHR const& getCapabilities() const { return m_capabilities; }
        std::vector<VkSurfaceFormatKHR> const& getFormats() const { return m_formats; }
        std::vector<VkPresentModeKHR> getPresentModes() const { return m_presentModes; }
        bool isPresentationSupported(std::size_t queueIndex) const;

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

        PhysicalDevice const& m_physicalDevice;
        Surface const& m_surface;
    };
}

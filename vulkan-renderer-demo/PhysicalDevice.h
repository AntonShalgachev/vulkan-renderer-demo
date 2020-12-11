#pragma once

#include "framework.h"

#include <optional>

namespace vkr
{
    class Surface;

    class PhysicalDevice
    {
    public:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool IsComplete()
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct PhysicalDeviceProperties
        {
            QueueFamilyIndices queueFamilyIndices;
            SwapchainSupportDetails swapchainSupportDetails;
        };

    public:
        PhysicalDevice(VkPhysicalDevice handle, Surface const& surface);

        VkPhysicalDeviceProperties const& getProperties() const;
        VkPhysicalDeviceFeatures const& getFeatures() const;

        VkPhysicalDevice const& getHandle() const { return m_handle; }

    private:
        mutable std::optional<VkPhysicalDeviceProperties> m_properties;
        mutable std::optional<VkPhysicalDeviceFeatures> m_features;

        VkPhysicalDevice m_handle;
    };
}

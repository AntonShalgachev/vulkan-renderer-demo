#pragma once

#include "framework.h"

#include <optional>

namespace vkr
{
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
        explicit PhysicalDevice(VkPhysicalDevice handle);

        VkPhysicalDeviceProperties const& getProperties() const { return m_properties; }
        VkPhysicalDeviceFeatures const& getFeatures() const { return m_features; }
        std::vector<VkQueueFamilyProperties> const& getQueueFamilyProperties() const { return m_queueFamilyProperties; }

        bool areExtensionsSupported(std::vector<char const*> const& requestedExtensions) const;

        VkPhysicalDevice const& getHandle() const { return m_handle; }

    private:
        void queryAvailableExtensions();
        void queryProperties();
        void queryFeatures();
        void queryQueueFamilyProperties();

    private:
        VkPhysicalDevice m_handle;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;

        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
    };
}

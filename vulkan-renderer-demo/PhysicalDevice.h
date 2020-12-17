#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice(VkPhysicalDevice handle);
        ~PhysicalDevice() = default;

        PhysicalDevice(PhysicalDevice const&) = delete;
        PhysicalDevice(PhysicalDevice&&);
        PhysicalDevice& operator=(PhysicalDevice const&) = delete;
        PhysicalDevice& operator=(PhysicalDevice&&) = delete;

        VkPhysicalDevice const& getHandle() const { return m_handle; }

        VkPhysicalDeviceProperties const& getProperties() const { return m_properties; }
        VkPhysicalDeviceFeatures const& getFeatures() const { return m_features; }
        std::vector<VkQueueFamilyProperties> const& getQueueFamilyProperties() const { return m_queueFamilyProperties; }

        bool areExtensionsSupported(std::vector<char const*> const& requestedExtensions) const;

    private:
        void init();

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

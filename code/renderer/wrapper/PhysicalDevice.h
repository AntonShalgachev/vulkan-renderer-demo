#pragma once

#include "QueueFamily.h"
#include "UniqueHandle.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

namespace vko
{
    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice(VkPhysicalDevice handle);

        VkPhysicalDevice getHandle() const { return m_handle; }

        VkPhysicalDeviceProperties const& getProperties() const { return m_properties; }
        VkPhysicalDeviceFeatures const& getFeatures() const { return m_features; }
        std::vector<QueueFamily> const& getQueueFamilies() const { return m_queueFamilies; }

        bool areExtensionsSupported(std::vector<char const*> const& requestedExtensions) const;

    private:
        void init();

        void queryAvailableExtensions();
        void queryProperties();
        void queryFeatures();
        void queryQueueFamilyProperties();

    private:
        UniqueHandle<VkPhysicalDevice> m_handle;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<std::string> m_availableExtensionNames;

        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;
        std::vector<QueueFamily> m_queueFamilies;
    };
}

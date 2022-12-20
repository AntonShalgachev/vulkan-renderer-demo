#pragma once

#include "renderer/wrapper/QueueFamily.h"
#include "renderer/wrapper/UniqueHandle.h"

#include "nstl/vector.h"
#include "nstl/string.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice(VkPhysicalDevice handle);

        VkPhysicalDevice getHandle() const { return m_handle; }

        VkPhysicalDeviceProperties const& getProperties() const { return m_properties; }
        VkPhysicalDeviceFeatures const& getFeatures() const { return m_features; }
        nstl::vector<QueueFamily> const& getQueueFamilies() const { return m_queueFamilies; } // TODO use std::span

        bool areExtensionsSupported(nstl::vector<char const*> const& requestedExtensions) const; // TODO use std::span

    private:
        void init();

        void queryAvailableExtensions();
        void queryProperties();
        void queryFeatures();
        void queryQueueFamilyProperties();

    private:
        UniqueHandle<VkPhysicalDevice> m_handle;

        nstl::vector<VkExtensionProperties> m_availableExtensions;
        nstl::vector<nstl::string> m_availableExtensionNames;

        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;
        nstl::vector<QueueFamily> m_queueFamilies;
    };
}

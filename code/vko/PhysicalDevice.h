#pragma once

#include "vko/QueueFamily.h"
#include "vko/UniqueHandle.h"

#include "nstl/vector.h"
#include "nstl/string.h"
#include "nstl/span.h"

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
        nstl::span<VkQueueFamilyProperties const> getQueueFamilyProperties() const { return m_queueFamilyProperties; }

        bool areExtensionsSupported(nstl::span<char const* const> requestedExtensions) const;

    private:
        UniqueHandle<VkPhysicalDevice> m_handle;

        nstl::vector<VkExtensionProperties> m_availableExtensions;

        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;
        nstl::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
    };
}

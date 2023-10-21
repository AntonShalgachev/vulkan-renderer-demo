#include "PhysicalDevice.h"

#include "vko/Assert.h"

#include "common/Utils.h"

vko::PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle)
{
    {
        uint32_t count = 0;
        VKO_VERIFY(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr));
        m_availableExtensions.resize(count);
        VKO_VERIFY(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_availableExtensions.data()));
    }

    vkGetPhysicalDeviceProperties(m_handle, &m_properties);
    vkGetPhysicalDeviceFeatures(m_handle, &m_features);

    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
        m_queueFamilyProperties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, m_queueFamilyProperties.data());
    }
}

bool vko::PhysicalDevice::areExtensionsSupported(nstl::span<char const* const> requestedExtensions) const
{
    return vkc::utils::hasEveryOption(m_availableExtensions, requestedExtensions, [](VkExtensionProperties const& props) { return props.extensionName; });
}

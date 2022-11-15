#include "PhysicalDevice.h"

#include "Assert.h"
#include "common/Utils.h"

vko::PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle)
{
    init();
}

bool vko::PhysicalDevice::areExtensionsSupported(nstl::vector<char const*> const& requestedExtensions) const
{
    return vkc::utils::hasEveryOption(m_availableExtensionNames, requestedExtensions);
}

void vko::PhysicalDevice::init()
{
    queryAvailableExtensions();
    queryProperties();
    queryFeatures();
    queryQueueFamilyProperties();
}

void vko::PhysicalDevice::queryAvailableExtensions()
{
    uint32_t count = 0;
    VKO_ASSERT(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr));
    m_availableExtensions.resize(count);
    VKO_ASSERT(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_availableExtensions.data()));

    for (const auto& extension : m_availableExtensions)
        m_availableExtensionNames.push_back(extension.extensionName);
}

void vko::PhysicalDevice::queryProperties()
{
    vkGetPhysicalDeviceProperties(m_handle, &m_properties);
}

void vko::PhysicalDevice::queryFeatures()
{
    vkGetPhysicalDeviceFeatures(m_handle, &m_features);
}

void vko::PhysicalDevice::queryQueueFamilyProperties()
{
    nstl::vector<VkQueueFamilyProperties> properties;

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, properties.data());

    m_queueFamilies.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        m_queueFamilies.emplace_back(i, properties[i]);
}

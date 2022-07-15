#include "PhysicalDevice.h"
#include "Utils.h"

vkr::PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle)
{
    init();
}

bool vkr::PhysicalDevice::areExtensionsSupported(std::vector<char const*> const& requestedExtensions) const
{
    return utils::hasEveryOption(m_availableExtensionNames, requestedExtensions);
}

void vkr::PhysicalDevice::init()
{
    queryAvailableExtensions();
    queryProperties();
    queryFeatures();
    queryQueueFamilyProperties();
}

void vkr::PhysicalDevice::queryAvailableExtensions()
{
    uint32_t count;
    VKR_ASSERT(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr));
    m_availableExtensions.resize(count);
    VKR_ASSERT(vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_availableExtensions.data()));

    for (const auto& extension : m_availableExtensions)
        m_availableExtensionNames.push_back(extension.extensionName);
}

void vkr::PhysicalDevice::queryProperties()
{
    vkGetPhysicalDeviceProperties(m_handle, &m_properties);
}

void vkr::PhysicalDevice::queryFeatures()
{
    vkGetPhysicalDeviceFeatures(m_handle, &m_features);
}

void vkr::PhysicalDevice::queryQueueFamilyProperties()
{
    std::vector<VkQueueFamilyProperties> properties;

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, properties.data());

    m_queueFamilies.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        m_queueFamilies.emplace_back(i, properties[i]);
}

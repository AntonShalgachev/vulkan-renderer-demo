#include "PhysicalDevice.h"
#include "Utils.h"

vkr::PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle)
{
    init();
}

vkr::PhysicalDevice::PhysicalDevice(PhysicalDevice&& other)
{
    std::swap(other.m_handle, m_handle);
    
    // swap data instead of initializing it
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
    vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr);
    m_availableExtensions.resize(count);
    vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_availableExtensions.data());

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
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
    m_queueFamilyProperties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, m_queueFamilyProperties.data());
}

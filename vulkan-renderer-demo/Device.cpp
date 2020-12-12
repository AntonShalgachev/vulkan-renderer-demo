#include "Device.h"
#include "PhysicalDevice.h"

vkr::Device::Device(PhysicalDevice const& physicalDevice, std::vector<const char*> const& extensions, uint32_t queueFamilyIndex)
{
    std::vector<float> queuePriorities = { 1.0f };

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = static_cast<uint32_t>(queuePriorities.size());
    queueCreateInfo.pQueuePriorities = queuePriorities.data();

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateDevice(physicalDevice.getHandle(), &deviceCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");
}

vkr::Device::~Device()
{
    vkDestroyDevice(m_handle, nullptr);
}

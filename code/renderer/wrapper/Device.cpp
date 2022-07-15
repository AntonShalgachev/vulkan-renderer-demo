#include "Device.h"
#include "PhysicalDevice.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "QueueFamilyIndices.h"
#include "Queue.h"
#include <set>
#include <stdexcept>

vkr::Device::Device(PhysicalDeviceSurfaceContainer const& physicalDeviceSurfaceContainer, std::vector<const char*> const& extensions)
{
    PhysicalDevice const& physicalDevice = physicalDeviceSurfaceContainer.getPhysicalDevice();

    QueueFamilyIndices const& indices = physicalDeviceSurfaceContainer.getParameters().getQueueFamilyIndices();

    std::set<QueueFamily const*> uniqueQueueFamilies = {&indices.getGraphicsQueueFamily(), &indices.getPresentQueueFamily()};

    // The device is created with 1 queue of each family

    std::vector<float> queuePriorities = { 1.0f };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (QueueFamily const* queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily->getIndex();
        queueCreateInfo.queueCount = static_cast<uint32_t>(queuePriorities.size());
        queueCreateInfo.pQueuePriorities = queuePriorities.data();
    }

    // TODO specify these features externally
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateDevice(physicalDevice.getHandle(), &deviceCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    for(QueueFamily const* queueFamily : uniqueQueueFamilies)
    {
        VkQueue handle = VK_NULL_HANDLE;
        vkGetDeviceQueue(m_handle, queueFamily->getIndex(), 0, &handle);
        Queue const& queue = m_queues.emplace_back(handle, *queueFamily);

        if (queueFamily->getIndex() == indices.getGraphicsQueueFamily().getIndex())
            m_graphicsQueue = &queue;
        if (queueFamily->getIndex() == indices.getPresentQueueFamily().getIndex())
            m_presentQueue = &queue;
    }

    if (m_graphicsQueue == nullptr || m_presentQueue == nullptr)
        throw std::runtime_error("failed to get device queues!");
}

void vkr::Device::waitIdle() const
{
    VKR_ASSERT(vkDeviceWaitIdle(m_handle));
}

vkr::Device::~Device()
{
    vkDestroyDevice(m_handle, nullptr);
}

#include "Device.h"

#include "vko/Assert.h"
#include "vko/PhysicalDevice.h"
#include "vko/Queue.h"

vko::Device::Device(vko::PhysicalDevice const& physicalDevice, uint32_t graphics, uint32_t present, nstl::span<const char* const> extensions)
{
    nstl::vector<uint32_t> uniqueQueueFamilies = { graphics };
    if (present != graphics)
        uniqueQueueFamilies.push_back(present);

    // The device is created with 1 queue of each family

    nstl::vector<float> queuePriorities = { 1.0f };

    nstl::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = static_cast<uint32_t>(queuePriorities.size());
        queueCreateInfo.pQueuePriorities = queuePriorities.data();
    }

    // TODO specify these features externally
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkPhysicalDeviceIndexTypeUint8FeaturesEXT indexTypeUint8Feature{};
    indexTypeUint8Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT;
    indexTypeUint8Feature.indexTypeUint8 = true;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &indexTypeUint8Feature;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    VKO_VERIFY(vkCreateDevice(physicalDevice.getHandle(), &deviceCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
    
    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkQueue handle = VK_NULL_HANDLE;
        vkGetDeviceQueue(m_handle, queueFamily, 0, &handle);
        Queue const& queue = m_queues.emplace_back(handle, queueFamily);

        if (queueFamily == graphics)
            m_graphicsQueue = &queue;
        if (queueFamily == present)
            m_presentQueue = &queue;
    }

    assert(m_graphicsQueue && m_presentQueue);
}

void vko::Device::waitIdle() const
{
    VKO_VERIFY(vkDeviceWaitIdle(m_handle));
}

vko::Device::~Device()
{
    vkDestroyDevice(m_handle, &m_allocator.getCallbacks());
}

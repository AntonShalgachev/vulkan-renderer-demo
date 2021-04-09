#include "DeviceMemory.h"

#include "PhysicalDevice.h"
#include "Device.h"
#include <stdexcept>

namespace
{
    uint32_t findMemoryType(vkr::PhysicalDevice const& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice.getHandle(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) == 0)
                continue;

            if ((memProperties.memoryTypes[i].propertyFlags & properties) != properties)
                continue;

            return i;
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
}

vkr::DeviceMemory::DeviceMemory(Application const& app, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties) : Object(app)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(getPhysicalDevice(), memoryRequirements.memoryTypeBits, memoryProperties);

    if (vkAllocateMemory(getDevice().getHandle(), &allocInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");
}

vkr::DeviceMemory::~DeviceMemory()
{
    vkFreeMemory(getDevice().getHandle(), m_handle, nullptr);
}

void vkr::DeviceMemory::copyFrom(void const* sourcePointer, std::size_t sourceSize)
{
    void* data;
    vkMapMemory(getDevice().getHandle(), m_handle, 0, sourceSize, 0, &data);
    memcpy(data, sourcePointer, sourceSize);
    vkUnmapMemory(getDevice().getHandle(), m_handle);
}

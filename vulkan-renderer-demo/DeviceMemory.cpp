#include "DeviceMemory.h"

#include "Renderer.h"
#include "PhysicalDevice.h"

namespace
{
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vkr::temp::getRenderer()->getPhysicalDevice().getHandle(), &memProperties);

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

vkr::DeviceMemory::DeviceMemory(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);

    if (vkAllocateMemory(temp::getDevice(), &allocInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");
}

vkr::DeviceMemory::~DeviceMemory()
{
    vkFreeMemory(temp::getDevice(), m_handle, nullptr);
}

void vkr::DeviceMemory::copyFrom(void const* sourcePointer, std::size_t sourceSize)
{
    void* data;
    vkMapMemory(temp::getDevice(), m_handle, 0, sourceSize, 0, &data);
    memcpy(data, sourcePointer, sourceSize);
    vkUnmapMemory(temp::getDevice(), m_handle);
}

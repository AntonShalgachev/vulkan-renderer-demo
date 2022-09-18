#include "DeviceMemory.h"

#include "PhysicalDevice.h"
#include "Device.h"
#include <stdexcept>
#include <cassert>

namespace
{
    uint32_t findMemoryType(vko::PhysicalDevice const& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

vko::DeviceMemory::DeviceMemory(Device const& device, PhysicalDevice const& physicalDevice, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties)
    : m_device(device)
    , m_requirements(memoryRequirements)
    , m_properties(memoryProperties)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);

    if (vkAllocateMemory(m_device.getHandle(), &allocInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    if ((memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        VKR_ASSERT(vkMapMemory(m_device.getHandle(), m_handle, 0, memoryRequirements.size, 0, &m_data));
}

vko::DeviceMemory::~DeviceMemory()
{
    if (!m_handle)
        return;

    if (m_data)
        vkUnmapMemory(m_device.getHandle(), m_handle);

    vkFreeMemory(m_device.getHandle(), m_handle, nullptr);
}

void vko::DeviceMemory::copyFrom(void const* sourcePointer, std::size_t sourceSize, std::size_t offset) const
{
    assert(m_data);
    assert(m_requirements.size - offset >= sourceSize);

    memcpy(static_cast<unsigned char*>(m_data) + offset, sourcePointer, sourceSize);
}

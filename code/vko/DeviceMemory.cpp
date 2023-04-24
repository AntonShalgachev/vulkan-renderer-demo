#include "DeviceMemory.h"

#include "vko/Assert.h"
#include "vko/PhysicalDevice.h"
#include "vko/Device.h"

#include <assert.h>

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

        assert(false);
        return 0;
    }
}

vko::DeviceMemory::DeviceMemory(Device const& device, PhysicalDevice const& physicalDevice, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties)
    : m_device(device.getHandle())
    , m_requirements(memoryRequirements)
    , m_properties(memoryProperties)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties);

    VKO_VERIFY(vkAllocateMemory(m_device, &allocInfo, m_allocator, &m_handle.get()));

    if ((memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        VKO_VERIFY(vkMapMemory(m_device, m_handle, 0, memoryRequirements.size, 0, &m_data));
}

vko::DeviceMemory::~DeviceMemory()
{
    if (!m_handle)
        return;

    if (m_data)
        vkUnmapMemory(m_device, m_handle);

    vkFreeMemory(m_device, m_handle, m_allocator);
}

void vko::DeviceMemory::copyFrom(void const* sourcePointer, size_t sourceSize, size_t offset) const
{
    assert(m_data);
    assert(offset + sourceSize <= m_requirements.size);

    memcpy(static_cast<unsigned char*>(m_data) + offset, sourcePointer, sourceSize);
}

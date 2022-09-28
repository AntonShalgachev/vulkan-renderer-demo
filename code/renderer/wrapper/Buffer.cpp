#include "Buffer.h"

#include "DeviceMemory.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "Device.h"
#include <cassert>

vko::Buffer::Buffer(Device const& device, VkDeviceSize size, VkBufferUsageFlags usage)
    : m_device(device.getHandle())
    , m_size(size)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
}

vko::Buffer::~Buffer()
{
    vkDestroyBuffer(m_device, m_handle, nullptr);
}

VkMemoryRequirements vko::Buffer::getMemoryRequirements() const
{
    // TODO cache
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, m_handle, &memoryRequirements);

    return memoryRequirements;
}

void vko::Buffer::bindMemory(DeviceMemory const& memory) const
{
    VKR_ASSERT(vkBindBufferMemory(m_device, m_handle, memory.getHandle(), 0));
}

void vko::Buffer::copy(VkCommandBuffer commandBuffer, Buffer const& source, std::size_t sourceOffset, Buffer const& destination, std::size_t destinationOffset, std::size_t size)
{
    assert(sourceOffset + size <= source.getSize());
    assert(destinationOffset + size <= destination.getSize());

    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = destinationOffset;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, source.getHandle(), destination.getHandle(), 1, &copyRegion);
}

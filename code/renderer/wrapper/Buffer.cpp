#include "Buffer.h"

#include "DeviceMemory.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "Device.h"
#include <stdexcept>

vkr::Buffer::Buffer(Device const& device, VkDeviceSize size, VkBufferUsageFlags usage)
    : m_device(device)
    , m_size(size)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device.getHandle(), &bufferCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
}

vkr::Buffer::~Buffer()
{
    vkDestroyBuffer(m_device.getHandle(), m_handle, nullptr);
}

VkMemoryRequirements vkr::Buffer::getMemoryRequirements() const
{
    // TODO cache
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device.getHandle(), m_handle, &memoryRequirements);

    return memoryRequirements;
}

void vkr::Buffer::bindMemory(DeviceMemory const& memory) const
{
    VKR_ASSERT(vkBindBufferMemory(m_device.getHandle(), m_handle, memory.getHandle(), 0));
}

void vkr::Buffer::copy(VkCommandBuffer commandBuffer, Buffer const& source, Buffer const& destination)
{
    if (source.getSize() != destination.getSize())
        throw std::runtime_error("Copy operation between buffers of different sizes");

    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = 0;
    copyRegion.srcOffset = 0;
    copyRegion.size = source.getSize();
    vkCmdCopyBuffer(commandBuffer, source.getHandle(), destination.getHandle(), 1, &copyRegion);
}

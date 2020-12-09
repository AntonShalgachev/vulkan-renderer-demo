#include "Buffer.h"

#include "DeviceMemory.h"
#include "ScopedOneTimeCommandBuffer.h"

vkr::Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
    m_size = size;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(temp::getDevice(), &bufferCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
}

vkr::Buffer::~Buffer()
{
    vkDestroyBuffer(temp::getDevice(), m_handle, nullptr);
}

VkMemoryRequirements vkr::Buffer::getMemoryRequirements() const
{
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(temp::getDevice(), m_handle, &memoryRequirements);

    return memoryRequirements;
}

void vkr::Buffer::bind(DeviceMemory const& memory) const
{
    vkBindBufferMemory(temp::getDevice(), m_handle, memory.getHandle(), 0);
}

void vkr::Buffer::copy(Buffer const& source, Buffer const& destination)
{
    if (source.getSize() != destination.getSize())
        throw std::runtime_error("Copy operation between buffers of different sizes");

    vkr::ScopedOneTimeCommandBuffer commandBuffer;

    VkBufferCopy copyRegion{};
    copyRegion.size = source.getSize();
    vkCmdCopyBuffer(commandBuffer.getHandle(), source.getHandle(), destination.getHandle(), 1, &copyRegion);
}

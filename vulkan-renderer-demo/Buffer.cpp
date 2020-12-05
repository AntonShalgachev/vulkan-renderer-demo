#include "Buffer.h"

#include "DeviceMemory.h"

vkr::Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(temp::getDevice(), &bufferCreateInfo, nullptr, &m_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
}

vkr::Buffer::~Buffer()
{
    vkDestroyBuffer(temp::getDevice(), m_buffer, nullptr);
}

VkMemoryRequirements vkr::Buffer::getMemoryRequirements() const
{
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(temp::getDevice(), m_buffer, &memoryRequirements);

    return memoryRequirements;
}

void vkr::Buffer::bind(DeviceMemory const& memory) const
{
    vkBindBufferMemory(temp::getDevice(), m_buffer, memory.getHandle(), 0);
}

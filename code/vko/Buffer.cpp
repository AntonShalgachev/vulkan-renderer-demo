#include "Buffer.h"

#include "vko/Assert.h"
#include "vko/DeviceMemory.h"
#include "vko/Device.h"

vko::Buffer::Buffer(Device const& device, VkDeviceSize size, VkBufferUsageFlags usage)
    : m_device(device.getHandle())
    , m_size(size)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VKO_VERIFY(vkCreateBuffer(m_device, &bufferCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::Buffer::~Buffer()
{
    if (m_handle)
        vkDestroyBuffer(m_device, m_handle, &m_allocator.getCallbacks());
}

VkMemoryRequirements vko::Buffer::getMemoryRequirements() const
{
    // TODO cache
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, m_handle, &memoryRequirements);

    return memoryRequirements;
}

void vko::Buffer::bindMemory(DeviceMemory const& memory, size_t offset) const
{
    VKO_VERIFY(vkBindBufferMemory(m_device, m_handle, memory.getHandle(), offset));
}

void vko::Buffer::copy(VkCommandBuffer commandBuffer, Buffer const& source, size_t sourceOffset, Buffer const& destination, size_t destinationOffset, size_t size)
{
    assert(sourceOffset + size <= source.getSize());
    assert(destinationOffset + size <= destination.getSize());

    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = destinationOffset;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, source.getHandle(), destination.getHandle(), 1, &copyRegion);
}

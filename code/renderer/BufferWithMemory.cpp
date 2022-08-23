#include "BufferWithMemory.h"

#include "Application.h"

vkr::BufferWithMemory::BufferWithMemory(Application const& app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : m_buffer(Buffer{ app.getDevice(), size, usage })
    , m_memory(DeviceMemory{ app.getDevice(), app.getPhysicalDevice(), m_buffer->getMemoryRequirements(), properties })
{
    m_buffer->bindMemory(*m_memory);
}

vkr::BufferWithMemory::~BufferWithMemory()
{
    // TODO remove this nasty hack
    m_buffer.reset();
    m_memory.reset();
}

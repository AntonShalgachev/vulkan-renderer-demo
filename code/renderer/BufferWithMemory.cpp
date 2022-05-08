#include "BufferWithMemory.h"

vkr::BufferWithMemory::BufferWithMemory(Application const& app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : m_buffer(app, size, usage)
    , m_memory(app, m_buffer.getMemoryRequirements(), properties)
{
    m_buffer.bindMemory(m_memory);
}

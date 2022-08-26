#include "BufferWithMemory.h"

#include "Application.h"

vkr::BufferWithMemory::BufferWithMemory(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : m_buffer(vko::Buffer{ device, size, usage })
    , m_memory(vko::DeviceMemory{ device, physicalDevice, m_buffer->getMemoryRequirements(), properties })
{
    m_buffer->bindMemory(*m_memory);
}

vkr::BufferWithMemory::~BufferWithMemory()
{
    // TODO remove this nasty hack
    m_buffer.reset();
    m_memory.reset();
}

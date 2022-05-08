#pragma once

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"

namespace vkr
{
    class Application;

    class BufferWithMemory
    {
    public:
        BufferWithMemory(Application const& app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        vkr::Buffer const& buffer() const { return m_buffer; }
        vkr::DeviceMemory const& memory() const { return m_memory; }

    private:
        vkr::Buffer m_buffer;
        vkr::DeviceMemory m_memory;
    };
}

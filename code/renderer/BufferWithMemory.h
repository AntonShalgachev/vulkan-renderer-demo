#pragma once

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"

#include <optional>

namespace vkr
{
    class Application;

    class BufferWithMemory
    {
    public:
        BufferWithMemory(Application const& app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~BufferWithMemory();

        BufferWithMemory(BufferWithMemory&& rhs) = default;
        BufferWithMemory& operator=(BufferWithMemory&& rhs) = default;

        vkr::Buffer const& buffer() const { return *m_buffer; }
        vkr::DeviceMemory const& memory() const { return *m_memory; }

    private:
        // TODO remove this nasty hack
        std::optional<vkr::Buffer> m_buffer;
        std::optional<vkr::DeviceMemory> m_memory;
    };
}

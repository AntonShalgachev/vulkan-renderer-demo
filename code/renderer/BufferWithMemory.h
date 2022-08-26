#pragma once

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"

#include <optional>

namespace vko
{
    class Device;
    class PhysicalDevice;
}

namespace vkr
{
    class Application;

    class BufferWithMemory
    {
    public:
        BufferWithMemory(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~BufferWithMemory();

        BufferWithMemory(BufferWithMemory&& rhs) = default;
        BufferWithMemory& operator=(BufferWithMemory&& rhs) = default;

        vko::Buffer const& buffer() const { return *m_buffer; }
        vko::DeviceMemory const& memory() const { return *m_memory; }

    private:
        // TODO remove this nasty hack
        std::optional<vko::Buffer> m_buffer;
        std::optional<vko::DeviceMemory> m_memory;
    };
}

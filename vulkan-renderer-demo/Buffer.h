#pragma once

#include "framework.h"

namespace vkr
{
    class DeviceMemory;

    class Buffer
    {
    public:
        Buffer(VkDeviceSize size, VkBufferUsageFlags usage);
        ~Buffer();

        VkMemoryRequirements getMemoryRequirements() const;
        void bind(DeviceMemory const& memory) const;

        VkBuffer getHandle() const { return m_buffer; }
        VkDeviceSize getSize() const { return m_size; }

    public:
        static void copy(Buffer const& source, Buffer const& destination);

    private:
        VkBuffer m_buffer;
        VkDeviceSize m_size;
    };
}

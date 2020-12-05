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

    private:
        VkBuffer m_buffer;
    };
}

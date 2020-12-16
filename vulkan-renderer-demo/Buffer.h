#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class DeviceMemory;

    class Buffer : public Object
    {
    public:
        explicit Buffer(VkDeviceSize size, VkBufferUsageFlags usage);
        ~Buffer();

        Buffer(Buffer const&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(Buffer const&) = delete;
        Buffer& operator=(Buffer&&) = delete;

        VkMemoryRequirements getMemoryRequirements() const;
        void bindMemory(DeviceMemory const& memory) const;

        VkBuffer getHandle() const { return m_handle; }
        VkDeviceSize getSize() const { return m_size; }

    public:
        static void copy(Buffer const& source, Buffer const& destination);

    private:
        VkBuffer m_handle = VK_NULL_HANDLE;
        VkDeviceSize m_size;
    };
}

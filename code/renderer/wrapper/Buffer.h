#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    class DeviceMemory;
    class Device;

    class Buffer
    {
    public:
        explicit Buffer(Device const& device, VkDeviceSize size, VkBufferUsageFlags usage);
        ~Buffer();

        Buffer(Buffer const&) = default;
        Buffer(Buffer&&) = default;
        Buffer& operator=(Buffer const&) = default;
        Buffer& operator=(Buffer&&) = default;

        VkMemoryRequirements getMemoryRequirements() const;
        void bindMemory(DeviceMemory const& memory) const;

        VkBuffer getHandle() const { return m_handle; }
        VkDeviceSize getSize() const { return m_size; }

    public:
        static void copy(VkCommandBuffer commandBuffer, Buffer const& source, Buffer const& destination);

    private:
        Device const& m_device;
        UniqueHandle<VkBuffer> m_handle;
        VkDeviceSize m_size;
    };
}

#pragma once

#include "renderer/wrapper/UniqueHandle.h"

#include <vulkan/vulkan.h>

#include <cstddef>

namespace vko
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
        static void copy(VkCommandBuffer commandBuffer, Buffer const& source, std::size_t sourceOffset, Buffer const& destination, std::size_t destinationOffset, std::size_t size);

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        UniqueHandle<VkBuffer> m_handle;
        VkDeviceSize m_size;
    };
}

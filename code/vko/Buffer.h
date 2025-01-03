#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

#include <cstddef>

namespace vko
{
    class DeviceMemory;
    class Device;

    class Buffer
    {
    public:
        explicit Buffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
        ~Buffer();

        Buffer(Buffer const&) = default;
        Buffer(Buffer&&) = default;
        Buffer& operator=(Buffer const&) = default;
        Buffer& operator=(Buffer&&) = default;

        VkMemoryRequirements getMemoryRequirements() const;
        void bindMemory(DeviceMemory const& memory, size_t offset = 0) const;

        VkBuffer getHandle() const { return m_handle; }
        VkDeviceSize getSize() const { return m_size; }

    public:
        static void copy(VkCommandBuffer commandBuffer, Buffer const& source, size_t sourceOffset, Buffer const& destination, size_t destinationOffset, size_t size);

    private:
        Allocator m_allocator{ AllocatorScope::Buffer };
        VkDevice m_device = VK_NULL_HANDLE;
        UniqueHandle<VkBuffer> m_handle;
        VkDeviceSize m_size;
    };
}

#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

#include <stddef.h>

namespace vko
{
    class Device;
    class QueueFamily;
    class CommandBuffers;

    class CommandPool
    {
    public:
    	explicit CommandPool(VkDevice device, QueueFamily const& queueFamily);
        ~CommandPool();

        vko::CommandBuffers allocate(size_t size) const;

        CommandPool(CommandPool const&) = default;
        CommandPool(CommandPool&&) = default;
        CommandPool& operator=(CommandPool const&) = default;
        CommandPool& operator=(CommandPool&&) = default;

        VkCommandPool getHandle() const { return m_handle; }

        void reset() const;

    private:
        Allocator m_allocator{ AllocatorScope::CommandPool };
        VkDevice m_device;

        UniqueHandle<VkCommandPool> m_handle;
    };
}

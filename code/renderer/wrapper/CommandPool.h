#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "UniqueHandle.h"

namespace vko
{
    class Device;
    class QueueFamily;
    class CommandBuffers;

    class CommandPool
    {
    public:
    	explicit CommandPool(Device const& device, QueueFamily const& queueFamily);
        ~CommandPool();

        vko::CommandBuffers createCommandBuffers(std::size_t size) const;

        CommandPool(CommandPool const&) = default;
        CommandPool(CommandPool&&) = default;
        CommandPool& operator=(CommandPool const&) = default;
        CommandPool& operator=(CommandPool&&) = default;

        VkCommandPool getHandle() const { return m_handle; }
        QueueFamily const& getQueueFamily() const { return m_queueFamily; }

        void reset() const;

    private:
        Device const& m_device;
        QueueFamily const& m_queueFamily;

        UniqueHandle<VkCommandPool> m_handle;
    };
}

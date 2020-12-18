#pragma once

#include <vulkan/vulkan.h>

namespace vkr
{
    class Device;
    class QueueFamily;

    class CommandPool
    {
    public:
    	explicit CommandPool(Device const& device, QueueFamily const& queueFamily);
    	~CommandPool();

        CommandPool(CommandPool const&) = delete;
        CommandPool(CommandPool&&) = delete;
        CommandPool& operator=(CommandPool const&) = delete;
        CommandPool& operator=(CommandPool&&) = delete;

        VkCommandPool const& getHandle() const { return m_handle; }
        QueueFamily const& getQueueFamily() const { return m_queueFamily; }

    private:
    	VkCommandPool m_handle = VK_NULL_HANDLE;

        Device const& m_device;
        QueueFamily const& m_queueFamily;
    };
}

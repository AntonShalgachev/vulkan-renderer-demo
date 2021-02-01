#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Object.h"

namespace vkr
{
    class Device;
    class QueueFamily;
    class CommandBuffer;
    class Queue;

    class CommandPool : vkr::Object
    {
    public:
    	explicit CommandPool(Application const& app);
    	~CommandPool();

        CommandBuffer createCommandBuffer() const;
        std::vector<CommandBuffer> createCommandBuffers(std::size_t size) const;

        CommandPool(CommandPool const&) = delete;
        CommandPool(CommandPool&&) = delete;
        CommandPool& operator=(CommandPool const&) = delete;
        CommandPool& operator=(CommandPool&&) = delete;

        VkCommandPool const& getHandle() const { return m_handle; }
        QueueFamily const& getQueueFamily() const { return m_queueFamily; }

    private:
    	VkCommandPool m_handle = VK_NULL_HANDLE;

        QueueFamily const& m_queueFamily;
    };
}

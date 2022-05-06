#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Object.h"
#include "UniqueHandle.h"

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

        CommandPool(CommandPool const&) = default;
        CommandPool(CommandPool&&) = default;
        CommandPool& operator=(CommandPool const&) = default;
        CommandPool& operator=(CommandPool&&) = default;

        VkCommandPool getHandle() const { return m_handle; }
        QueueFamily const& getQueueFamily() const { return m_queueFamily; }

        void reset() const;

    private:
        UniqueHandle<VkCommandPool> m_handle;

        QueueFamily const& m_queueFamily;
    };
}

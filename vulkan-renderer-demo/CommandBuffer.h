#pragma once

#include <vulkan/vulkan.h>
#include <memory>

namespace vkr
{
    class CommandBuffers;
    class Semaphore;
    class Fence;
    class Queue;

    class CommandBuffer
    {
    public:
    	CommandBuffer(std::shared_ptr<CommandBuffers> const& container, std::size_t index);

        VkCommandBuffer const& getHandle() const;

        void begin(VkCommandBufferUsageFlags flags) const;
        void end() const;
        void submit(Queue const& queue, Semaphore const* signalSemaphore, Semaphore const* waitSemaphore, Fence const* signalFence) const;

    private:
        std::shared_ptr<CommandBuffers> m_container;
        std::size_t m_index;
    };
}

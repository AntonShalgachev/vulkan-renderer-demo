#pragma once

#include <vulkan/vulkan.h>
#include <memory>

namespace vko
{
    class CommandBuffers;
    class Semaphore;
    class Fence;
    class Queue;
}

namespace vkr
{
    class CommandBuffer
    {
    public:
    	CommandBuffer(std::shared_ptr<vko::CommandBuffers> const& container, std::size_t index);

        VkCommandBuffer const& getHandle() const;

        void reset() const;
        void begin(bool oneTime = true) const;
        void end() const;
        void submit(vko::Queue const& queue, vko::Semaphore const* signalSemaphore, vko::Semaphore const* waitSemaphore, vko::Fence const* signalFence) const;

    private:
        std::shared_ptr<vko::CommandBuffers> m_container;
        std::size_t m_index;
    };
}

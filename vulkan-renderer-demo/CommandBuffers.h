#pragma once

#include "framework.h"

namespace vkr
{
    class Semaphore;
    class Fence;

    // TODO make it responsible for a single command buffer
    class CommandBuffers
    {
    public:
    	explicit CommandBuffers(std::size_t size);
    	~CommandBuffers();

        CommandBuffers(CommandBuffers const&) = delete;
        CommandBuffers(CommandBuffers&&) = delete;
        CommandBuffers& operator=(CommandBuffers const&) = delete;
        CommandBuffers& operator=(CommandBuffers&&) = delete;

        void begin(std::size_t index, VkCommandBufferUsageFlags flags);
        void end(std::size_t index);
        void submitAndWait(std::size_t index);
        void submit(std::size_t index, Semaphore const& signalSemaphore, Semaphore const& waitSemaphore, Fence const& signalFence);

        std::size_t getSize() const { return m_handles.size(); }
        VkCommandBuffer const& getHandle(std::size_t index) const { return m_handles[index]; }

    private:
        std::vector<VkCommandBuffer> m_handles;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>
#include <vector>
#include "UniqueHandle.h"

namespace vko
{
    class CommandPool;
    class Device;
    class Queue;
    class Semaphore;
    class Fence;

    class CommandBuffers
    {
        friend class CommandPool;

    public:
    	CommandBuffers(Device const& device, CommandPool const& commandPool, std::size_t size);
    	~CommandBuffers();

        CommandBuffers(CommandBuffers const&) = default;
        CommandBuffers(CommandBuffers&&) = default;
        CommandBuffers& operator=(CommandBuffers const&) = default;
        CommandBuffers& operator=(CommandBuffers&&) = default;

        std::size_t getSize() const { return m_handles.size(); }
        VkCommandBuffer getHandle(std::size_t index) const { return m_handles[index]; }

        void reset(std::size_t index) const;
        void begin(std::size_t index, bool oneTime = true) const;
        void end(std::size_t index) const;
        void submit(std::size_t index, Queue const& queue, Semaphore const* signalSemaphore, Semaphore const* waitSemaphore, Fence const* signalFence) const;

    private:
        VkDevice m_device;
        VkCommandPool m_commandPool;

        std::vector<UniqueHandle<VkCommandBuffer>> m_handles; // TODO fix this ugliness
    };
}

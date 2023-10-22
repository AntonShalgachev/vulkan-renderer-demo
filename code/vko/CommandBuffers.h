#pragma once

#include "vko/UniqueHandle.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

#include <cstddef>

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
    	CommandBuffers(VkDevice device, VkCommandPool commandPool, size_t size);
    	~CommandBuffers();

        CommandBuffers(CommandBuffers const&) = default;
        CommandBuffers(CommandBuffers&&) = default;
        CommandBuffers& operator=(CommandBuffers const&) = default;
        CommandBuffers& operator=(CommandBuffers&&) = default;

        size_t getSize() const { return m_handles.size(); }
        VkCommandBuffer const& getHandle(size_t index) const { return m_handles[index].get(); }

        void reset(size_t index) const;
        void begin(size_t index, bool oneTime = true) const;
        void end(size_t index) const;
        void submit(size_t index, VkQueue queue, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore, VkFence signalFence) const;

    private:
        VkDevice m_device;
        VkCommandPool m_commandPool;

        nstl::vector<UniqueHandle<VkCommandBuffer>> m_handles; // TODO fix this ugliness
    };
}

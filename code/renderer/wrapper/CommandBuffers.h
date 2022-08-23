#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>
#include <vector>

namespace vkr
{
    class CommandPool;
    class Device;

    class CommandBuffers
    {
        friend class CommandPool;

    public:
    	CommandBuffers(Device const& device, CommandPool const& commandPool, std::size_t size);
    	~CommandBuffers();

        CommandBuffers(CommandBuffers const&) = delete;
        CommandBuffers(CommandBuffers&&) = delete;
        CommandBuffers& operator=(CommandBuffers const&) = delete;
        CommandBuffers& operator=(CommandBuffers&&) = delete;

        std::size_t getSize() const { return m_handles.size(); }
        VkCommandBuffer const& getHandle(std::size_t index) const { return m_handles[index]; }

    private:
        Device const& m_device;
        CommandPool const& m_commandPool;

        std::vector<VkCommandBuffer> m_handles; // TODO use UniqueHandle
    };
}

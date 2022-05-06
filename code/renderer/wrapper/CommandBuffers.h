#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>
#include <vector>
#include "Object.h"

namespace vkr
{
    class CommandPool;

    class CommandBuffers : public Object
    {
        friend class CommandPool;

    public:
    	CommandBuffers(Application const& app, CommandPool const& commandPool, std::size_t size);
    	~CommandBuffers();

        CommandBuffers(CommandBuffers const&) = delete;
        CommandBuffers(CommandBuffers&&) = delete;
        CommandBuffers& operator=(CommandBuffers const&) = delete;
        CommandBuffers& operator=(CommandBuffers&&) = delete;

        std::size_t getSize() const { return m_handles.size(); }
        VkCommandBuffer const& getHandle(std::size_t index) const { return m_handles[index]; }

    private:
        CommandPool const& m_commandPool;

        std::vector<VkCommandBuffer> m_handles;
    };
}

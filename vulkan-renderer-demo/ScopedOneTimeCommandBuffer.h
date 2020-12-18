#pragma once

#include <vulkan/vulkan.h>

#include "CommandBuffers.h"
#include "Object.h"

namespace vkr
{
    // TODO move to utils namespace
    class ScopedOneTimeCommandBuffer : Object
    {
    public:
    	ScopedOneTimeCommandBuffer(Application const& app);
    	~ScopedOneTimeCommandBuffer();

        ScopedOneTimeCommandBuffer(ScopedOneTimeCommandBuffer const&) = delete;
        ScopedOneTimeCommandBuffer(ScopedOneTimeCommandBuffer&&) = delete;
        ScopedOneTimeCommandBuffer& operator=(ScopedOneTimeCommandBuffer const&) = delete;
        ScopedOneTimeCommandBuffer& operator=(ScopedOneTimeCommandBuffer&&) = delete;

        VkCommandBuffer const& getHandle() const { return m_commandBuffers.getHandle(0); }

    private:
        CommandBuffers m_commandBuffers;
    };
}

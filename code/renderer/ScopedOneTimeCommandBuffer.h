#pragma once

#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "Object.h"
#include "wrapper/CommandBuffers.h"

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

        VkCommandBuffer getHandle() const { return m_commandBuffers.getHandle(0); }

        void submit();

    private:
        vko::CommandBuffers m_commandBuffers;

        bool m_submitted = false;
    };
}

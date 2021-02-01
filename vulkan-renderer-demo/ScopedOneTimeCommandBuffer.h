#pragma once

#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
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

        VkCommandBuffer const& getHandle() const { return m_commandBuffer.getHandle(); }

        void submit();

    private:
        CommandBuffer m_commandBuffer;

        bool m_submitted = false;
    };
}

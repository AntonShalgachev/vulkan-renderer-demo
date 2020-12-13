#pragma once

#include "framework.h"

#include "CommandBuffers.h"

namespace vkr
{
    // TODO move to utils namespace
    class ScopedOneTimeCommandBuffer
    {
    public:
    	ScopedOneTimeCommandBuffer();
    	~ScopedOneTimeCommandBuffer();

        ScopedOneTimeCommandBuffer(ScopedOneTimeCommandBuffer const&) = delete;
        ScopedOneTimeCommandBuffer(ScopedOneTimeCommandBuffer&&) = delete;
        ScopedOneTimeCommandBuffer& operator=(ScopedOneTimeCommandBuffer const&) = delete;
        ScopedOneTimeCommandBuffer& operator=(ScopedOneTimeCommandBuffer&&) = delete;

        VkCommandBuffer const& getHandle() const { return m_commandBuffers.getHandle(0); }

    private:
        CommandBuffers m_commandBuffers{ 1 };
    };
}

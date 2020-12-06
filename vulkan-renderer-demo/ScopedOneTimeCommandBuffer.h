#pragma once

#include "framework.h"

#include "CommandBuffers.h"

namespace vkr
{
    class ScopedOneTimeCommandBuffer
    {
    public:
    	ScopedOneTimeCommandBuffer();
    	~ScopedOneTimeCommandBuffer();

        VkCommandBuffer const& getHandle() const { return m_commandBuffers.getHandle(0); }

    private:
        CommandBuffers m_commandBuffers{ 1 };
    };
}
#pragma once

#include "framework.h"

namespace vkr
{
    class Fence
    {
    public:
    	Fence();
    	~Fence();

        void wait();
        void reset();

        VkFence const& getHandle() const { return m_handle; }

    private:
    	VkFence m_handle;
    };
}

#pragma once

#include "framework.h"

namespace vkr
{
    class Fence
    {
    public:
    	Fence();
    	~Fence();

        Fence(Fence const&) = delete;
        Fence(Fence&&) = default;
        Fence& operator=(Fence const&) = delete;
        Fence& operator=(Fence&&) = default;

        void wait();
        void reset();

        VkFence const& getHandle() const { return m_handle; }

    private:
    	VkFence m_handle = VK_NULL_HANDLE;
    };
}

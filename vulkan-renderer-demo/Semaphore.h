#pragma once

#include "framework.h"

namespace vkr
{
    class Semaphore
    {
    public:
    	Semaphore();
    	~Semaphore();

        VkSemaphore const& getHandle() const { return m_handle; }

    private:
    	VkSemaphore m_handle = VK_NULL_HANDLE;
    };
}

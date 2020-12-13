#pragma once

#include "framework.h"

namespace vkr
{
    class Semaphore
    {
    public:
    	Semaphore();
    	~Semaphore();

        Semaphore(Semaphore const&) = delete;
        Semaphore(Semaphore&&) = default;
        Semaphore& operator=(Semaphore const&) = delete;
        Semaphore& operator=(Semaphore&&) = default;

        VkSemaphore const& getHandle() const { return m_handle; }

    private:
    	VkSemaphore m_handle = VK_NULL_HANDLE;
    };
}

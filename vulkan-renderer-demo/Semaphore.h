#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"

namespace vkr
{
    class Semaphore : public Object
    {
    public:
    	Semaphore(Application const& app);
    	~Semaphore();

        Semaphore(Semaphore const&) = delete;
        Semaphore(Semaphore&&);
        Semaphore& operator=(Semaphore const&) = delete;
        Semaphore& operator=(Semaphore&&) = delete;

        VkSemaphore const& getHandle() const { return m_handle; }

    private:
    	VkSemaphore m_handle = VK_NULL_HANDLE;
    };
}

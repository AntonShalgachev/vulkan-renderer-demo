#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class Semaphore : public Object
    {
    public:
    	Semaphore(Application const& app);
    	~Semaphore();

        Semaphore(Semaphore const&) = default;
        Semaphore(Semaphore&&) = default;
        Semaphore& operator=(Semaphore const&) = default;
        Semaphore& operator=(Semaphore&&) = default;

        VkSemaphore getHandle() const { return m_handle; }

    private:
    	UniqueHandle<VkSemaphore> m_handle;
    };
}

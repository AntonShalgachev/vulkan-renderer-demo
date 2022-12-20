#pragma once

#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    class Semaphore
    {
    public:
    	Semaphore(Device const& device);
    	~Semaphore();

        Semaphore(Semaphore const&) = default;
        Semaphore(Semaphore&&) = default;
        Semaphore& operator=(Semaphore const&) = default;
        Semaphore& operator=(Semaphore&&) = default;

        VkSemaphore getHandle() const { return m_handle; }

    private:
        Device const& m_device;
    	UniqueHandle<VkSemaphore> m_handle;
    };
}

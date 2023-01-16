#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

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
        Allocator m_allocator{ AllocatorScope::Semaphore };
        Device const& m_device;
    	UniqueHandle<VkSemaphore> m_handle;
    };
}

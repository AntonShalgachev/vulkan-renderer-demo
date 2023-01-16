#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    class Fence
    {
    public:
    	Fence(Device const& device);
    	~Fence();

        Fence(Fence const&) = default;
        Fence(Fence&&) = default;
        Fence& operator=(Fence const&) = default;
        Fence& operator=(Fence&&) = default;

        void wait() const;
        void reset() const;
        bool isSignaled() const;

        VkFence getHandle() const { return m_handle; }

    private:
        Allocator m_allocator{ AllocatorScope::Fence };
        Device const& m_device;
    	UniqueHandle<VkFence> m_handle;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

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
        Device const& m_device;
    	UniqueHandle<VkFence> m_handle;
    };
}

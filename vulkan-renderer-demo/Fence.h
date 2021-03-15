#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class Fence : public Object
    {
    public:
    	Fence(Application const& app);
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
    	UniqueHandle<VkFence> m_handle;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"

namespace vkr
{
    class Fence : public Object
    {
    public:
    	Fence(Application const& app);
    	~Fence();

        Fence(Fence const&) = delete;
        Fence(Fence&&);
        Fence& operator=(Fence const&) = delete;
        Fence& operator=(Fence&&) = delete;

        void wait() const;
        void reset() const;

        VkFence const& getHandle() const { return m_handle; }

    private:
    	VkFence m_handle = VK_NULL_HANDLE;
    };
}

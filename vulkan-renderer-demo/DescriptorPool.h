#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorPool
    {
    public:
    	DescriptorPool(std::size_t size);
    	~DescriptorPool();

        VkDescriptorPool const& getHandle() const { return m_handle; }

    private:
    	VkDescriptorPool m_handle;
    };
}

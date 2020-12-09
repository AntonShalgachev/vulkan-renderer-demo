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
        std::size_t getSize() const { return m_size; }

    private:
    	VkDescriptorPool m_handle = VK_NULL_HANDLE;
        std::size_t m_size;
    };
}

#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorSetLayout
    {
    public:
    	DescriptorSetLayout();
    	~DescriptorSetLayout();

        VkDescriptorSetLayout const& getHandle() const { return m_handle; }

    private:
    	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
    };
}

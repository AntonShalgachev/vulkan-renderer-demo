#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorSetLayout
    {
    public:
    	explicit DescriptorSetLayout();
    	~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout const&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&&) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

        VkDescriptorSetLayout const& getHandle() const { return m_handle; }

    private:
    	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
    };
}

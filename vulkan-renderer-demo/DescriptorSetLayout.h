#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"

namespace vkr
{
    class DescriptorSetLayout : public Object
    {
    public:
    	explicit DescriptorSetLayout(Application const& app);
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

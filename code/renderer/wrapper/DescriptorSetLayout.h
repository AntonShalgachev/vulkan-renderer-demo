#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class DescriptorSetLayout : public Object
    {
    public:
    	explicit DescriptorSetLayout(Application const& app);
    	~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout const&) = default;
        DescriptorSetLayout(DescriptorSetLayout&&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout const&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

        VkDescriptorSetLayout getHandle() const { return m_handle; }

    private:
    	UniqueHandle<VkDescriptorSetLayout> m_handle;
    };
}

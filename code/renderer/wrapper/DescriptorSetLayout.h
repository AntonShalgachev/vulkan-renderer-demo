#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class DescriptorSetLayout : public Object
    {
    public:
    	explicit DescriptorSetLayout(Application const& app, bool hasSampler);
    	~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout const&) = default;
        DescriptorSetLayout(DescriptorSetLayout&&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout const&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

        VkDescriptorSetLayout getHandle() const { return m_handle; }
        bool hasSampler() const { return m_hasSampler; }

    private:
    	UniqueHandle<VkDescriptorSetLayout> m_handle;
        bool m_hasSampler = false;
    };
}

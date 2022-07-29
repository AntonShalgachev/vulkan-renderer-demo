#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    // TODO rename to something like "Uniform metadata"
    struct DescriptorSetConfiguration
    {
        bool hasTexture = false;
        bool hasNormalMap = false;

        auto operator<=>(DescriptorSetConfiguration const& rhs) const = default;
    };

    class DescriptorSetLayout : public Object
    {
    public:
    	explicit DescriptorSetLayout(Application const& app, DescriptorSetConfiguration const& config);
    	~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout const&) = default;
        DescriptorSetLayout(DescriptorSetLayout&&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout const&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

        VkDescriptorSetLayout getHandle() const { return m_handle; }
        DescriptorSetConfiguration getConfiguration() const { return m_configuration; }

    private:
    	UniqueHandle<VkDescriptorSetLayout> m_handle;
        DescriptorSetConfiguration m_configuration;
    };
}

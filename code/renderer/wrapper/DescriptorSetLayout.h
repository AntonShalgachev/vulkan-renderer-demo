#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vko
{
    class Device;

    // TODO rename to something like "Uniform metadata"
    struct DescriptorSetConfiguration
    {
        bool hasBuffer = false;
        bool hasTexture = false;
        bool hasNormalMap = false;

        auto operator<=>(DescriptorSetConfiguration const& rhs) const = default;
    };

    class DescriptorSetLayout
    {
    public:
    	explicit DescriptorSetLayout(Device const& device, DescriptorSetConfiguration config);
    	~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout const&) = default;
        DescriptorSetLayout(DescriptorSetLayout&&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout const&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

        VkDescriptorSetLayout getHandle() const { return m_handle; }
        DescriptorSetConfiguration getConfiguration() const { return m_configuration; }

    private:
        Device const& m_device;

    	UniqueHandle<VkDescriptorSetLayout> m_handle;
        DescriptorSetConfiguration m_configuration;
    };
}

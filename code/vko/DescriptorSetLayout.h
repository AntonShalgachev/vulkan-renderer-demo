#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

#include <compare> // TODO avoid including this header?

namespace vko
{
    class Device;

    // TODO rename to something like "Uniform metadata"
    struct DescriptorSetConfiguration
    {
        bool hasBuffer = false;
        bool hasTexture = false;
        bool hasNormalMap = false;
        bool hasShadowMap = false;

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
        Allocator m_allocator{ AllocatorScope::DescriptorSetLayout };
        Device const& m_device;

    	UniqueHandle<VkDescriptorSetLayout> m_handle;
        DescriptorSetConfiguration m_configuration;
    };
}

#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include "nstl/vector.h"
#include "nstl/span.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    class PipelineLayout
    {
    public:
    	explicit PipelineLayout(Device const& device, nstl::span<VkDescriptorSetLayout const> setLayouts, nstl::span<VkPushConstantRange const> pushConstantRanges);
    	~PipelineLayout();

        PipelineLayout(PipelineLayout const&) = default;
        PipelineLayout(PipelineLayout&&) = default;
        PipelineLayout& operator=(PipelineLayout const&) = default;
        PipelineLayout& operator=(PipelineLayout&&) = default;

        VkPipelineLayout getHandle() const { return m_handle; }
        nstl::span<VkDescriptorSetLayout const> getDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

    private:
        Allocator m_allocator{ AllocatorScope::PipelineLayout };
        Device const& m_device;
    	UniqueHandle<VkPipelineLayout> m_handle;

        nstl::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    };
}

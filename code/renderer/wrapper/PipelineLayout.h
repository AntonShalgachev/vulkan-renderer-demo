#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vko
{
    class Device;

    class DescriptorSetLayout;

    class PipelineLayout
    {
    public:
    	explicit PipelineLayout(Device const& device, DescriptorSetLayout const* descriptorSetLayout, std::size_t pushConstantsSize);
    	~PipelineLayout();

        PipelineLayout(PipelineLayout const&) = default;
        PipelineLayout(PipelineLayout&&) = default;
        PipelineLayout& operator=(PipelineLayout const&) = default;
        PipelineLayout& operator=(PipelineLayout&&) = default;

        VkPipelineLayout getHandle() const { return m_handle; }

    private:
        Device const& m_device;
    	UniqueHandle<VkPipelineLayout> m_handle;
    };
}

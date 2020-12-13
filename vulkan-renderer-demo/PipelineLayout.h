#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorSetLayout;

    class PipelineLayout
    {
    public:
    	explicit PipelineLayout(DescriptorSetLayout const& descriptorSetLayout);
    	~PipelineLayout();

        PipelineLayout(PipelineLayout const&) = delete;
        PipelineLayout(PipelineLayout&&) = delete;
        PipelineLayout& operator=(PipelineLayout const&) = delete;
        PipelineLayout& operator=(PipelineLayout&&) = delete;

        VkPipelineLayout const& getHandle() const { return m_handle; }

    private:
    	VkPipelineLayout m_handle = VK_NULL_HANDLE;
    };
}

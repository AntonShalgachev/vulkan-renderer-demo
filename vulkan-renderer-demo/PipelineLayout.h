#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorSetLayout;

    class PipelineLayout
    {
    public:
    	PipelineLayout(DescriptorSetLayout const& descriptorSetLayout);
    	~PipelineLayout();

        VkPipelineLayout const& getHandle() const { return m_handle; }

    private:
    	VkPipelineLayout m_handle;
    };
}

#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class DescriptorSetLayout;

    class PipelineLayout : public Object
    {
    public:
    	explicit PipelineLayout(Application const& app, DescriptorSetLayout const& descriptorSetLayout);
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

#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class DescriptorSetLayout;

    class PipelineLayout : public Object
    {
    public:
    	explicit PipelineLayout(Application const& app, DescriptorSetLayout const& descriptorSetLayout, std::size_t pushConstantsSize);
    	~PipelineLayout();

        PipelineLayout(PipelineLayout const&) = default;
        PipelineLayout(PipelineLayout&&) = default;
        PipelineLayout& operator=(PipelineLayout const&) = default;
        PipelineLayout& operator=(PipelineLayout&&) = default;

        VkPipelineLayout getHandle() const { return m_handle; }

    private:
    	UniqueHandle<VkPipelineLayout> m_handle;
    };
}

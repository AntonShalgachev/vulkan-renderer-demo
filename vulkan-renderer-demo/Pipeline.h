#pragma once

#include "framework.h"

namespace vkr
{
    class ShaderModule;
    class PipelineLayout;
    class RenderPass;

    class Pipeline
    {
    public:
    	Pipeline(PipelineLayout const& layout, RenderPass const& renderPass, VkExtent2D extent, ShaderModule const& vertShaderModule, ShaderModule const& fragShaderModule);
    	~Pipeline();

        VkPipeline const& getHandle() const { return m_handle; }

    private:
    	VkPipeline m_handle;
    };
}

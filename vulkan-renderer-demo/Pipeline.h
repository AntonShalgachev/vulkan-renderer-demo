#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class ShaderModule;
    class PipelineLayout;
    class RenderPass;

    class Pipeline : public Object
    {
    public:
    	explicit Pipeline(Application const& app, PipelineLayout const& layout, RenderPass const& renderPass, VkExtent2D extent, ShaderModule const& vertShaderModule, ShaderModule const& fragShaderModule);
    	~Pipeline();

        Pipeline(Pipeline const&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline const&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;

        VkPipeline const& getHandle() const { return m_handle; }

    private:
    	VkPipeline m_handle = VK_NULL_HANDLE;
    };
}

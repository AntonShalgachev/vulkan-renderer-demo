#pragma once

#include <vulkan/vulkan.h>
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

        void bind(VkCommandBuffer commandBuffer) const;

        Pipeline(Pipeline const&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline const&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;

        VkPipeline const& getHandle() const { return m_handle; }

    public:
        static void resetBoundPipeline() { ms_boundPipeline = nullptr; }

    private:
    	VkPipeline m_handle = VK_NULL_HANDLE;

    private:
        static Pipeline const* ms_boundPipeline;
    };
}

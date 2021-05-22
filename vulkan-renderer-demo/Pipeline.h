#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

// TODO maybe remove?
#include "Shader.h"

namespace vkr
{
    class ShaderModule;
    class PipelineLayout;
    class RenderPass;
    class VertexLayout;

    class Pipeline : public Object
    {
    public:
    	explicit Pipeline(Application const& app, PipelineLayout const& layout, RenderPass const& renderPass, VkExtent2D extent, Shader::Key const& shaderKey, VertexLayout const& vertexLayout);
    	~Pipeline();

        void bind(VkCommandBuffer commandBuffer) const;

        Pipeline(Pipeline const&) = default;
        Pipeline(Pipeline&&) = default;
        Pipeline& operator=(Pipeline const&) = default;
        Pipeline& operator=(Pipeline&&) = default;

        VkPipeline getHandle() const { return m_handle; }

    public:
        static void resetBoundPipeline() { ms_boundPipeline = nullptr; }

    private:
    	UniqueHandle<VkPipeline> m_handle;

    private:
        static Pipeline const* ms_boundPipeline;
    };
}

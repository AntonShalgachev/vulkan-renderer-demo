#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    struct PipelineConfiguration;
}

namespace vko
{
    class ShaderModule;
    class PipelineLayout;
    class RenderPass;
    class VertexLayoutDescriptions;
    class Device;

    class Pipeline
    {
    public:
    	explicit Pipeline(Device const& device, vkr::PipelineConfiguration const& config);
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
        Device const& m_device;
    	UniqueHandle<VkPipeline> m_handle;

    private:
        static Pipeline const* ms_boundPipeline; // TODO remove this hack
    };
}

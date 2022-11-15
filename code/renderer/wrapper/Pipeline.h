#pragma once

#include "UniqueHandle.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

#include <span>

namespace vko
{
    class ShaderModule;
    class PipelineLayout;
    class RenderPass;
    class Device;

    class Pipeline
    {
    public:
        struct Config
        {
            // TODO replace with the simplier vertex layout struct
            nstl::vector<VkVertexInputBindingDescription> bindingDescriptions;
            nstl::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            bool cullBackFaces = true;
            bool wireframe = false;
            bool depthTest = true;
            bool alphaBlending = false;
        };

    	explicit Pipeline(Device const& device, PipelineLayout const& layout, RenderPass const& renderPass, std::span<ShaderModule const*> shaderModules, Config const& config);
    	~Pipeline();

        void bind(VkCommandBuffer commandBuffer) const;

        Pipeline(Pipeline const&) = default;
        Pipeline(Pipeline&&) = default;
        Pipeline& operator=(Pipeline const&) = default;
        Pipeline& operator=(Pipeline&&) = default;

        VkPipeline getHandle() const { return m_handle; }

        VkPipelineLayout getPipelineLayoutHandle() const { return m_pipelineLayout; }
        std::span<VkDescriptorSetLayout const> getDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

    private:
        Device const& m_device;
    	UniqueHandle<VkPipeline> m_handle;

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        nstl::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    };
}

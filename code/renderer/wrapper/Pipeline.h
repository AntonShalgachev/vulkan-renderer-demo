#pragma once

#include "UniqueHandle.h"

#include <vulkan/vulkan.h>

#include <vector>
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
            VkExtent2D extent{ 0, 0 };

            // TODO replace with the simplier vertex layout struct
            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            bool cullBackFaces = true;
            bool wireframe = false;
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
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    };
}

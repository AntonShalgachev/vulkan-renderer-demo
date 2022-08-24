#include "Pipeline.h"

#include "ShaderModule.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "Device.h"
#include "VertexLayout.h"

#include <stdexcept>
#include <iterator>
#include <algorithm>

namespace
{
	std::vector<VkPipelineShaderStageCreateInfo> createStageDescriptions(std::vector<vko::ShaderModule> const& shaderModules)
	{
        std::vector<VkPipelineShaderStageCreateInfo> stageDescriptions;
		stageDescriptions.reserve(shaderModules.size());
        std::transform(shaderModules.begin(), shaderModules.end(), std::back_inserter(stageDescriptions),
            [](vko::ShaderModule const& shaderModule) { return shaderModule.createStageCreateInfo(); });
        return stageDescriptions;
	}

	VkPipelineVertexInputStateCreateInfo initVertexInputCreateInfo(std::vector<VkVertexInputBindingDescription> const& bindingDescriptions, std::vector<VkVertexInputAttributeDescription> const& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

		vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		return vertexInputCreateInfo;
	}

    VkPipelineInputAssemblyStateCreateInfo initInputAssemblyCreateInfo()
    {
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        return inputAssemblyCreateInfo;
    }

    VkViewport initViewport(VkExtent2D const& extent)
    {
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

        return viewport;
    }

    VkRect2D initScissor(VkExtent2D const& extent)
    {
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

        return scissor;
    }

    VkPipelineViewportStateCreateInfo initViewportStateCreateInfo(VkViewport const& viewport, VkRect2D const& scissor)
    {
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;

        return viewportStateCreateInfo;
    }

    VkPipelineRasterizationStateCreateInfo initRasterizerCreateInfo(bool cullBackFaces, bool wireframe)
    {
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
		rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizerCreateInfo.lineWidth = 1.0f;
		rasterizerCreateInfo.cullMode = cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizerCreateInfo.depthBiasClamp = 0.0f;
		rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

        return rasterizerCreateInfo;
    }

    VkPipelineMultisampleStateCreateInfo initMultisamplingCreateInfo()
    {
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
		multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingCreateInfo.minSampleShading = 1.0f;
		multisamplingCreateInfo.pSampleMask = nullptr;
		multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

        return multisamplingCreateInfo;
    }

    VkPipelineColorBlendAttachmentState initColorBlendAttachment()
    {
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        return colorBlendAttachment;
    }

    VkPipelineColorBlendStateCreateInfo initColorBlendingCreateInfo(VkPipelineColorBlendAttachmentState const& colorBlendAttachment)
    {
		VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
		colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendingCreateInfo.attachmentCount = 1;
		colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;
		colorBlendingCreateInfo.blendConstants[0] = 0.0f;
		colorBlendingCreateInfo.blendConstants[1] = 0.0f;
		colorBlendingCreateInfo.blendConstants[2] = 0.0f;
		colorBlendingCreateInfo.blendConstants[3] = 0.0f;

        return colorBlendingCreateInfo;
    }

    VkPipelineDepthStencilStateCreateInfo initDepthStencilCreateInfo()
    {
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.depthTestEnable = VK_TRUE;
		depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
		depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

        return depthStencilCreateInfo;
	}
}

vko::Pipeline::Pipeline(Device const& device, PipelineLayout const& layout, RenderPass const& renderPass, std::vector<ShaderModule> const& shaderModules, Config const& config)
	: m_device(device)
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = createStageDescriptions(shaderModules);

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = initVertexInputCreateInfo(config.bindingDescriptions, config.attributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = initInputAssemblyCreateInfo();

    VkViewport viewport = initViewport(config.extent);
    VkRect2D scissor = initScissor(config.extent);
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = initViewportStateCreateInfo(viewport, scissor);

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = initRasterizerCreateInfo(config.cullBackFaces, config.wireframe);

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = initMultisamplingCreateInfo();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = initDepthStencilCreateInfo();

	VkPipelineColorBlendAttachmentState colorBlendAttachment = initColorBlendAttachment();
	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = initColorBlendingCreateInfo(colorBlendAttachment);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = layout.getHandle();
    pipelineCreateInfo.renderPass = renderPass.getHandle();
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_device.getHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");
}

vko::Pipeline::~Pipeline()
{
    vkDestroyPipeline(m_device.getHandle(), m_handle, nullptr);
}

void vko::Pipeline::bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle);
}

#include "Pipeline.h"

#include "vko/ShaderModule.h"
#include "vko/PipelineLayout.h"
#include "vko/RenderPass.h"
#include "vko/Device.h"
#include "vko/Assert.h"

namespace
{
	nstl::vector<VkPipelineShaderStageCreateInfo> createStageDescriptions(nstl::span<vko::ShaderModule const*> shaderModules)
	{
        nstl::vector<VkPipelineShaderStageCreateInfo> stageDescriptions;
		stageDescriptions.reserve(shaderModules.size());
		for (vko::ShaderModule const* shaderModule : shaderModules)
			stageDescriptions.push_back(shaderModule->createStageCreateInfo());
        return stageDescriptions;
	}

	VkPipelineVertexInputStateCreateInfo initVertexInputCreateInfo(nstl::vector<VkVertexInputBindingDescription> const& bindingDescriptions, nstl::vector<VkVertexInputAttributeDescription> const& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO configure externally
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        return inputAssemblyCreateInfo;
    }

    VkPipelineViewportStateCreateInfo initViewportStateCreateInfo()
    {
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = nullptr;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = nullptr;

        return viewportStateCreateInfo;
    }

    VkPipelineRasterizationStateCreateInfo initRasterizerCreateInfo(bool cullBackFaces, bool wireframe, bool depthBias)
    {
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
		rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizerCreateInfo.lineWidth = 1.0f;
		rasterizerCreateInfo.cullMode = cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		if (depthBias)
		{
			rasterizerCreateInfo.depthBiasEnable = VK_TRUE;
			rasterizerCreateInfo.depthBiasConstantFactor = 4.0f;
			rasterizerCreateInfo.depthBiasClamp = 0.0f;
			rasterizerCreateInfo.depthBiasSlopeFactor = 1.5f;
		}

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

    VkPipelineColorBlendAttachmentState initColorBlendAttachment(bool alphaBlending)
    {
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = alphaBlending ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
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

    VkPipelineDepthStencilStateCreateInfo initDepthStencilCreateInfo(bool depthTest)
    {
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		if (depthTest)
		{
            depthStencilCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
		}

        return depthStencilCreateInfo;
	}
}

vko::Pipeline::Pipeline(Device const& device, PipelineLayout const& layout, RenderPass const& renderPass, nstl::span<ShaderModule const*> shaderModules, Config const& config)
	: m_device(device)
	, m_pipelineLayout(layout.getHandle())
{
	nstl::vector<VkPipelineShaderStageCreateInfo> shaderStages = createStageDescriptions(shaderModules);

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = initVertexInputCreateInfo(config.bindingDescriptions, config.attributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = initInputAssemblyCreateInfo();

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = initViewportStateCreateInfo();

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = initRasterizerCreateInfo(config.cullBackFaces, config.wireframe, config.depthBias);

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = initMultisamplingCreateInfo();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = initDepthStencilCreateInfo(config.depthTest);

	VkPipelineColorBlendAttachmentState colorBlendAttachment = initColorBlendAttachment(config.alphaBlending);
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = initColorBlendingCreateInfo(colorBlendAttachment);

	nstl::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_VIEWPORT,
	};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

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
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = layout.getHandle();
    pipelineCreateInfo.renderPass = renderPass.getHandle();
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

	VKO_VERIFY(vkCreateGraphicsPipelines(m_device.getHandle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));

	auto layoutsSpan = layout.getDescriptorSetLayouts();
	m_descriptorSetLayouts = nstl::vector<VkDescriptorSetLayout>{ layoutsSpan.begin(), layoutsSpan.end() };
}

vko::Pipeline::~Pipeline()
{
    if (m_handle)
		vkDestroyPipeline(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}

void vko::Pipeline::bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle);
}

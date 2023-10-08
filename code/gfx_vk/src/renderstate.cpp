#include "renderstate.h"

#include "context.h"
#include "shader.h"

#include "vko/Device.h"
#include "vko/Assert.h"

namespace
{
    struct temporary_resources
    {
        nstl::vector<VkVertexInputBindingDescription> binding_descriptions;
        nstl::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        nstl::vector<VkDynamicState> dynamic_states;
    };

    VkFormat get_attribute_type(gfx::attribute_type type)
    {
        switch (type)
        {
        case gfx::attribute_type::vec2f:
            return VK_FORMAT_R32G32_SFLOAT;
        case gfx::attribute_type::vec3f:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case gfx::attribute_type::vec4f:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case gfx::attribute_type::uint32:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            assert(false);
        }

        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }

    VkPipelineVertexInputStateCreateInfo create_vertex_input_state(gfx_vk::renderstate_init_params const& params, temporary_resources& resources)
    {
        for (gfx::buffer_binding_description const& desc : params.vertex_config.buffer_bindings)
        {
            resources.binding_descriptions.push_back({
                .binding = static_cast<uint32_t>(desc.buffer_index),
                .stride = static_cast<uint32_t>(desc.stride),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            });
        }

        for (gfx::attribute_description const& desc : params.vertex_config.attributes)
        {
            resources.attribute_descriptions.push_back({
                .location = static_cast<uint32_t>(desc.location),
                .binding = static_cast<uint32_t>(desc.buffer_binding_index),
                .format = get_attribute_type(desc.type),
                .offset = static_cast<uint32_t>(desc.offset),
            });
        }

        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

            .vertexBindingDescriptionCount = static_cast<uint32_t>(resources.binding_descriptions.size()),
            .pVertexBindingDescriptions = resources.binding_descriptions.data(),

            .vertexAttributeDescriptionCount = static_cast<uint32_t>(resources.attribute_descriptions.size()),
            .pVertexAttributeDescriptions = resources.attribute_descriptions.data(),
        };
    }

    VkPipelineInputAssemblyStateCreateInfo create_input_assembly_state()
    {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TODO configure externally
            .primitiveRestartEnable = VK_FALSE,
        };
    }

    VkPipelineViewportStateCreateInfo create_viewport_state()
    {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = nullptr, // state is dynamic
            .scissorCount = 1,
            .pScissors = nullptr, // state is dynamic
        };
    }

    VkPipelineRasterizationStateCreateInfo create_rasterization_state(gfx_vk::renderstate_init_params const& params)
    {
        VkPipelineRasterizationStateCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = params.flags.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
            .cullMode = static_cast<VkCullModeFlags>(params.flags.cull_backfaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE),
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f,
        };

        if (params.flags.depth_bias)
        {
            info.depthBiasEnable = VK_TRUE;
            info.depthBiasConstantFactor = 4.0f;
            info.depthBiasClamp = 0.0f;
            info.depthBiasSlopeFactor = 1.5f;
        }

        return info;
    }

    VkPipelineMultisampleStateCreateInfo create_multisample_state()
    {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };
    }

    VkPipelineDepthStencilStateCreateInfo create_depth_stencil_state(gfx_vk::renderstate_init_params const& params)
    {
        VkPipelineDepthStencilStateCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };

        if (params.flags.depth_test)
        {
            info.depthTestEnable = VK_TRUE;
            info.depthWriteEnable = VK_TRUE;
            info.depthCompareOp = VK_COMPARE_OP_LESS;
            info.depthBoundsTestEnable = VK_FALSE;
        }

        return info;
    }

    VkPipelineColorBlendStateCreateInfo create_color_blending_state(gfx_vk::renderstate_init_params const& params, temporary_resources& resources)
    {
        resources.color_blend_attachment = {
            .blendEnable = params.flags.alpha_blending ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &resources.color_blend_attachment,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        };
    }

    VkPipelineDynamicStateCreateInfo create_dynamic_state(temporary_resources& resources)
    {
        resources.dynamic_states = {
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_VIEWPORT,
        };

        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(resources.dynamic_states.size()),
            .pDynamicStates = resources.dynamic_states.data(),
        };
    }
}

gfx_vk::renderstate::renderstate(context& context, renderstate_init_params const& params)
    : m_context(context)
    , m_params(renderstate_init_params_storage::from_view(params))
{
    temporary_resources resources;

    nstl::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    for (gfx::shader_handle shader : params.shaders)
        shader_stages.push_back(m_context.get_resources().get_shader(shader).create_stage_create_info());
    
    VkPipelineVertexInputStateCreateInfo vertex_input = create_vertex_input_state(params, resources);
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = create_input_assembly_state();
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = create_viewport_state();
    VkPipelineRasterizationStateCreateInfo rasterization = create_rasterization_state(params);
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = create_multisample_state();
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = create_depth_stencil_state(params);
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = create_color_blending_state(params, resources);
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = create_dynamic_state(resources);

    VkGraphicsPipelineCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterization,
        .pMultisampleState = &multisamplingCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &colorBlendingCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = params.layout,
        .renderPass = params.renderpass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VKO_VERIFY(vkCreateGraphicsPipelines(m_context.get_device().getHandle(), VK_NULL_HANDLE, 1, &info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::renderstate::~renderstate()
{
    if (!m_handle)
        return;

    vkDestroyPipeline(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}

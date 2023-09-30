#include "renderpass.h"

#include "context.h"
#include "conversions.h"

#include "vko/Device.h"
#include "vko/Assert.h"

gfx_vk::renderpass::renderpass(context& context, gfx::renderpass_params const& params)
    : m_context(context)
{
    nstl::vector<VkAttachmentDescription> attachments;
    nstl::vector<VkAttachmentReference> color_attachment_refs;
    nstl::optional<VkAttachmentReference> depth_stencil_attachment_ref;

    for (gfx::image_format format : params.color_attachment_formats)
    {
        uint32_t attachment_index = static_cast<uint32_t>(attachments.size());

        // TODO review layouts
        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkImageLayout final_layout = params.has_presentable_images ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments.push_back({
            .format = utils::get_format(format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initial_layout,
            .finalLayout = final_layout,
        });

        color_attachment_refs.push_back(VkAttachmentReference{
            .attachment = attachment_index,
            .layout = layout,
        });
    }

    if (params.depth_stencil_attachment_format)
    {
        uint32_t attachment_index = static_cast<uint32_t>(attachments.size());

        // TODO review layouts
        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkImageLayout final_layout = params.has_presentable_images ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments.push_back(VkAttachmentDescription{
            .format = utils::get_format(*params.depth_stencil_attachment_format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = params.keep_depth_values_after_renderpass ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initial_layout,
            .finalLayout = final_layout,
        });

        depth_stencil_attachment_ref = VkAttachmentReference{
            .attachment = attachment_index,
            .layout = layout,
        };
    }

    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(color_attachment_refs.size()),
        .pColorAttachments = color_attachment_refs.data(),
        .pDepthStencilAttachment = depth_stencil_attachment_ref ? &*depth_stencil_attachment_ref : nullptr,
    };

    VkSubpassDependency dependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VKO_VERIFY(vkCreateRenderPass(m_context.get_device().getHandle(), &renderPassCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::renderpass::~renderpass()
{
    if (!m_handle)
        return;

    vkDestroyRenderPass(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}

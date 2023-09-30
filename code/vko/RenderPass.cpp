#include "RenderPass.h"

#include "vko/Device.h"
#include "vko/Assert.h"

#include "nstl/static_vector.h"

vko::RenderPass::RenderPass(Device const& device, nstl::optional<VkFormat> colorFormat, nstl::optional<VkFormat> depthFormat, VkImageLayout finalDepthLayout, bool keepDepthValuesAfterRenderPass) : m_device(device)
{
    nstl::static_vector<VkAttachmentDescription, 2> attachments;
    nstl::static_vector<VkAttachmentReference, 1> colorAttachmentRefs;
    nstl::optional<VkAttachmentReference> depthStencilAttachmentRef;

    if (colorFormat)
    {
        colorAttachmentRefs.push_back(VkAttachmentReference{
            .attachment = static_cast<uint32_t>(attachments.size()),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });

        attachments.push_back(VkAttachmentDescription{
            .format = *colorFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        });
    }
    if (depthFormat)
    {
        depthStencilAttachmentRef = VkAttachmentReference{
            .attachment = static_cast<uint32_t>(attachments.size()),
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        attachments.push_back(VkAttachmentDescription{
            .format = *depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = keepDepthValuesAfterRenderPass ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = finalDepthLayout,
        });
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    assert(colorAttachmentRefs.size() <= UINT32_MAX);
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    subpass.pDepthStencilAttachment = depthStencilAttachmentRef ? &*depthStencilAttachmentRef : nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VKO_VERIFY(vkCreateRenderPass(m_device.getHandle(), &renderPassCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::RenderPass::~RenderPass()
{
    vkDestroyRenderPass(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}

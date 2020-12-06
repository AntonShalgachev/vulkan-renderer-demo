#include "Framebuffer.h"

#include "RenderPass.h"
#include "Swapchain.h"
#include "ImageView.h"

vkr::Framebuffer::Framebuffer(vkr::ImageView const& colorImageView, vkr::ImageView const& depthImageView, vkr::RenderPass const& renderPass, VkExtent2D extent)
{
    std::array<VkImageView, 2> attachments = { colorImageView.getHandle(), depthImageView.getHandle() };

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass.getHandle();
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(temp::getDevice(), &framebufferCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
}

vkr::Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(temp::getDevice(), m_handle, nullptr);
}

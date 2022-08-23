#include "Framebuffer.h"

#include <array>

#include "RenderPass.h"
#include "Swapchain.h"
#include "ImageView.h"
#include "Device.h"
#include <stdexcept>

vko::Framebuffer::Framebuffer(Device const& device, vko::ImageView const& colorImageView, vko::ImageView const& depthImageView, vko::RenderPass const& renderPass, VkExtent2D extent)
    : m_device(device)
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

    if (vkCreateFramebuffer(m_device.getHandle(), &framebufferCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
}

vko::Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(m_device.getHandle(), m_handle, nullptr);
}

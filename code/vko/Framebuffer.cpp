#include "Framebuffer.h"

#include "vko/RenderPass.h"
#include "vko/Swapchain.h"
#include "vko/ImageView.h"
#include "vko/Device.h"
#include "vko/Assert.h"

#include "nstl/array.h"

vko::Framebuffer::Framebuffer(Device const& device, vko::ImageView const& colorImageView, vko::ImageView const& depthImageView, vko::RenderPass const& renderPass, VkExtent2D extent)
    : m_device(device)
{
    nstl::array<VkImageView, 2> attachments = { colorImageView.getHandle(), depthImageView.getHandle() };

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass.getHandle();
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;

    VKO_ASSERT(vkCreateFramebuffer(m_device.getHandle(), &framebufferCreateInfo, nullptr, &m_handle.get()));
}

vko::Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(m_device.getHandle(), m_handle, nullptr);
}

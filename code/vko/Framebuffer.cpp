#include "Framebuffer.h"

#include "vko/RenderPass.h"
#include "vko/Swapchain.h"
#include "vko/ImageView.h"
#include "vko/Device.h"
#include "vko/Assert.h"

#include "nstl/static_vector.h"

vko::Framebuffer::Framebuffer(Device const& device, vko::ImageView const* colorImageView, vko::ImageView const* depthImageView, VkRenderPass renderPass, VkExtent2D extent)
    : m_device(device)
{
    nstl::static_vector<VkImageView, 2> attachments;

    if (colorImageView)
        attachments.push_back(colorImageView->getHandle());
    if (depthImageView)
        attachments.push_back(depthImageView->getHandle());

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;

    VKO_VERIFY(vkCreateFramebuffer(m_device.getHandle(), &framebufferCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}

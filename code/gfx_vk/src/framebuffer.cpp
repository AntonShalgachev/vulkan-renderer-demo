#include "framebuffer.h"

#include "context.h"
#include "image.h"
#include "renderpass.h"

gfx_vk::framebuffer::framebuffer(context& context, gfx::framebuffer_params const& params)
    : m_context(context)
{
    size_t width = 0;
    size_t height = 0;

    nstl::vector<VkImageView> attachments;
    for (gfx::image_handle attachment : params.attachments)
    {
        image const& vk_image = m_context.get_resources().get_image(attachment);
        if (width > 0)
            assert(vk_image.get_width() == width);
        if (height > 0)
            assert(vk_image.get_height() == height);

        width = vk_image.get_width();
        height = vk_image.get_height();

        attachments.push_back(vk_image.get_view_handle());
        m_attachment_types.push_back(vk_image.get_type());
    }

    VkFramebufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_context.get_resources().get_renderpass(params.renderpass).get_handle(),
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .layers = 1,
    };

    GFX_VK_VERIFY(vkCreateFramebuffer(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));

    m_extent = { info.width, info.height };
}

gfx_vk::framebuffer::~framebuffer()
{
    if (!m_handle)
        return;

    vkDestroyFramebuffer(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
    m_handle = nullptr;
}

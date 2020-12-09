#pragma once

#include "framework.h"

namespace vkr
{
    class RenderPass;
    class ImageView;

    class Framebuffer
    {
    public:
        Framebuffer(vkr::ImageView const& colorImageView, vkr::ImageView const& depthImageView, vkr::RenderPass const& renderPass, VkExtent2D extent);
        ~Framebuffer();

        VkFramebuffer getHandle() const { return m_handle; }

    private:
        VkFramebuffer m_handle = VK_NULL_HANDLE;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"

namespace vkr
{
    class RenderPass;
    class ImageView;

    class Framebuffer : public Object
    {
    public:
        explicit Framebuffer(Application const& app, vkr::ImageView const& colorImageView, vkr::ImageView const& depthImageView, vkr::RenderPass const& renderPass, VkExtent2D extent);
        ~Framebuffer();

        Framebuffer(Framebuffer const&) = delete;
        Framebuffer(Framebuffer&&) = delete;
        Framebuffer& operator=(Framebuffer const&) = delete;
        Framebuffer& operator=(Framebuffer&&) = delete;

        VkFramebuffer getHandle() const { return m_handle; }

    private:
        VkFramebuffer m_handle = VK_NULL_HANDLE;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class RenderPass;
    class ImageView;

    class Framebuffer : public Object
    {
    public:
        explicit Framebuffer(Application const& app, vkr::ImageView const& colorImageView, vkr::ImageView const& depthImageView, vkr::RenderPass const& renderPass, VkExtent2D extent);
        ~Framebuffer();

        Framebuffer(Framebuffer const&) = default;
        Framebuffer(Framebuffer&&) = default;
        Framebuffer& operator=(Framebuffer const&) = default;
        Framebuffer& operator=(Framebuffer&&) = default;

        VkFramebuffer getHandle() const { return m_handle; }

    private:
        UniqueHandle<VkFramebuffer> m_handle;
    };
}

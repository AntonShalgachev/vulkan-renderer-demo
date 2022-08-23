#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    class RenderPass;
    class ImageView;
    class Device;

    class Framebuffer
    {
    public:
        explicit Framebuffer(Device const& device, vkr::ImageView const& colorImageView, vkr::ImageView const& depthImageView, vkr::RenderPass const& renderPass, VkExtent2D extent);
        ~Framebuffer();

        Framebuffer(Framebuffer const&) = default;
        Framebuffer(Framebuffer&&) = default;
        Framebuffer& operator=(Framebuffer const&) = default;
        Framebuffer& operator=(Framebuffer&&) = default;

        VkFramebuffer getHandle() const { return m_handle; }

    private:
        Device const& m_device;
        UniqueHandle<VkFramebuffer> m_handle;
    };
}

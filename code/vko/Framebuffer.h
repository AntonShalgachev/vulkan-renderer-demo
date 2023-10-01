#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class RenderPass;
    class ImageView;
    class Device;

    class Framebuffer
    {
    public:
        explicit Framebuffer(Device const& device, vko::ImageView const* colorImageView, vko::ImageView const* depthImageView, VkRenderPass renderPass, VkExtent2D extent);
        ~Framebuffer();

        Framebuffer(Framebuffer const&) = default;
        Framebuffer(Framebuffer&&) = default;
        Framebuffer& operator=(Framebuffer const&) = default;
        Framebuffer& operator=(Framebuffer&&) = default;

        VkFramebuffer getHandle() const { return m_handle; }

    private:
        Allocator m_allocator{ AllocatorScope::Framebuffer };
        Device const& m_device;
        UniqueHandle<VkFramebuffer> m_handle;
    };
}

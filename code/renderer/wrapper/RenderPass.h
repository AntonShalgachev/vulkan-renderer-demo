#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vkr
{
    class Swapchain;
    class Device;

    class RenderPass
    {
    public:
        explicit RenderPass(Device const& device, Swapchain const& swapchain, VkFormat depthFormat);
        ~RenderPass();

        RenderPass(RenderPass const&) = default;
        RenderPass(RenderPass&&) = default;
        RenderPass& operator=(RenderPass const&) = default;
        RenderPass& operator=(RenderPass&&) = default;

        VkRenderPass getHandle() const { return m_handle; }
        VkFormat getDepthFormat() const { return m_depthFormat; }

    private:
        Device const& m_device;
        UniqueHandle<VkRenderPass> m_handle;
        VkFormat m_depthFormat;
    };
}

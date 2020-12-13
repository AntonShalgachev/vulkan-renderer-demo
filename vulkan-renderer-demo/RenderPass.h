#pragma once

#include "framework.h"

namespace vkr
{
    class Swapchain;

    class RenderPass
    {
    public:
        explicit RenderPass(Swapchain const& swapchain);
        ~RenderPass();

        RenderPass(RenderPass const&) = delete;
        RenderPass(RenderPass&&) = delete;
        RenderPass& operator=(RenderPass const&) = delete;
        RenderPass& operator=(RenderPass&&) = delete;

        VkRenderPass getHandle() const { return m_handle; }
        VkFormat getDepthFormat() const { return m_depthFormat; }

    private:
        VkRenderPass m_handle = VK_NULL_HANDLE;
        VkFormat m_depthFormat;
    };
}

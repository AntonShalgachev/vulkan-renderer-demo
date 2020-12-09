#pragma once

#include "framework.h"

namespace vkr
{
    class Swapchain;

    class RenderPass
    {
    public:
        RenderPass(Swapchain const& swapchain);
        ~RenderPass();

        VkRenderPass getHandle() const { return m_handle; }
        VkFormat getDepthFormat() const { return m_depthFormat; }

    private:
        VkRenderPass m_handle = VK_NULL_HANDLE;
        VkFormat m_depthFormat;
    };
}

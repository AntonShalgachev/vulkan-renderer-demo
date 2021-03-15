#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class Swapchain;

    class RenderPass : public Object
    {
    public:
        explicit RenderPass(Application const& app, Swapchain const& swapchain);
        ~RenderPass();

        RenderPass(RenderPass const&) = default;
        RenderPass(RenderPass&&) = default;
        RenderPass& operator=(RenderPass const&) = default;
        RenderPass& operator=(RenderPass&&) = default;

        VkRenderPass getHandle() const { return m_handle; }
        VkFormat getDepthFormat() const { return m_depthFormat; }

    private:
        UniqueHandle<VkRenderPass> m_handle;
        VkFormat m_depthFormat;
    };
}

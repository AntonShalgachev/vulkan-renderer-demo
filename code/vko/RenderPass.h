#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    class RenderPass
    {
    public:
        explicit RenderPass(Device const& device, VkFormat colorFormat, VkFormat depthFormat);
        ~RenderPass();

        RenderPass(RenderPass const&) = default;
        RenderPass(RenderPass&&) = default;
        RenderPass& operator=(RenderPass const&) = default;
        RenderPass& operator=(RenderPass&&) = default;

        VkRenderPass getHandle() const { return m_handle; }

    private:
        Allocator m_allocator{ AllocatorScope::RenderPass };
        Device const& m_device;
        UniqueHandle<VkRenderPass> m_handle;
    };
}

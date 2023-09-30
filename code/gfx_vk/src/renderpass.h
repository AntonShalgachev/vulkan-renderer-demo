#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class renderpass final : public gfx::renderpass
    {
    public:
        renderpass(context& context, gfx::renderpass_params const& params);
        ~renderpass();

        VkRenderPass get_handle() const { return m_handle; }

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::RenderPass };
        vko::UniqueHandle<VkRenderPass> m_handle;
    };
}

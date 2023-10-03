#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class framebuffer final
    {
    public:
        framebuffer(context& context, gfx::framebuffer_params const& params);
        ~framebuffer();

        VkFramebuffer get_handle() const { return m_handle; }

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::Framebuffer };
        vko::UniqueHandle<VkFramebuffer> m_handle;
    };
}

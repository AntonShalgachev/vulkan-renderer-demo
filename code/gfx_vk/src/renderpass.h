#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class renderpass final
    {
    public:
        renderpass(context& context, gfx::renderpass_params const& params);
        ~renderpass();

        VkRenderPass get_handle() const { return m_handle; }

    private:
        context& m_context;

        unique_handle<VkRenderPass> m_handle;
    };
}

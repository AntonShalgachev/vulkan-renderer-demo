#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;
    class shader;

    struct renderstate_init_params
    {
        nstl::span<gfx::shader* const> shaders;
        gfx::vertex_configuration vertex_config;
        gfx::renderstate_flags flags;

        VkPipelineLayout layout;
        VkRenderPass renderpass;
    };

    class renderstate final : public gfx::renderstate
    {
    public:
        renderstate(context& context, renderstate_init_params const& params);
        ~renderstate();

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::Pipeline };
        vko::UniqueHandle<VkPipeline> m_handle;
    };
}

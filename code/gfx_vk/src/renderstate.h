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
        nstl::span<gfx::shader_handle const> shaders;
        gfx::vertex_configuration_view vertex_config;
        gfx::renderstate_flags flags;

        VkPipelineLayout layout;
        VkRenderPass renderpass;

        bool operator==(renderstate_init_params const& rhs) const = default;
    };

    struct renderstate_init_params_storage
    {
        nstl::vector<gfx::shader_handle> shaders;
        gfx::vertex_configuration_storage vertex_config;
        gfx::renderstate_flags flags;

        VkPipelineLayout layout;
        VkRenderPass renderpass;

        static renderstate_init_params_storage from_view(renderstate_init_params const& view)
        {
            return {
                .shaders = {view.shaders.begin(), view.shaders.end()},
                .vertex_config = gfx::vertex_configuration_storage::from_view(view.vertex_config),
                .flags = view.flags,

                .layout = view.layout,
                .renderpass = view.renderpass,
            };
        }

        operator renderstate_init_params() const
        {
            return {
                .shaders = shaders,
                .vertex_config = vertex_config,
                .flags = flags,

                .layout = layout,
                .renderpass = renderpass,
            };
        }
    };

    class renderstate final
    {
    public:
        renderstate(context& context, renderstate_init_params const& params);
        ~renderstate();

        renderstate_init_params get_params() const { return m_params; }
        VkPipeline get_handle() const { return m_handle; }

    private:
        context& m_context;
        renderstate_init_params_storage m_params;

        vko::Allocator m_allocator{ vko::AllocatorScope::Pipeline };
        vko::UniqueHandle<VkPipeline> m_handle;
    };
}

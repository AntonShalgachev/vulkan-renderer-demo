#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class shader final
    {
    public:
        shader(context& context, gfx::shader_params const& params);
        ~shader();

        VkPipelineShaderStageCreateInfo create_stage_create_info() const;

    private:
        context& m_context;

        gfx::shader_stage m_stage;
        nstl::string m_entry_point;

        unique_handle<VkShaderModule> m_handle;
    };
}

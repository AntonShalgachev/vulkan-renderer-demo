#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

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

        vko::Allocator m_allocator{ vko::AllocatorScope::ShaderModule };
        vko::UniqueHandle<VkShaderModule> m_handle;
    };
}

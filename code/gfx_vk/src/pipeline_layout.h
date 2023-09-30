#pragma once

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class pipeline_layout final
    {
    public:
        pipeline_layout(context& context, nstl::span<VkDescriptorSetLayout const> set_layouts);
        ~pipeline_layout();

        VkPipelineLayout get_handle() const { return m_handle; }

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::PipelineLayout };
        vko::UniqueHandle<VkPipelineLayout> m_handle;
    };
}

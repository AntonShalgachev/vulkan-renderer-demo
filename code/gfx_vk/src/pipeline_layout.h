#pragma once

#include "utils.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class pipeline_layout final
    {
    public:
        pipeline_layout(context& context, nstl::span<VkDescriptorSetLayout const> layouts);
        ~pipeline_layout();

        nstl::span<VkDescriptorSetLayout const> get_layouts() { return m_layouts; }

        VkPipelineLayout get_handle() const { return m_handle; }

    private:
        context& m_context;
        nstl::vector<VkDescriptorSetLayout> m_layouts;

        unique_handle<VkPipelineLayout> m_handle;
    };
}

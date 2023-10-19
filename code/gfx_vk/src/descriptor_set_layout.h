#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class descriptor_set_layout final
    {
    public:
        descriptor_set_layout(context& context, gfx::descriptorgroup_layout_view const& layout);
        ~descriptor_set_layout();

        gfx::descriptorgroup_layout_view get_layout() const { return m_layout; }

        VkDescriptorSetLayout get_handle() const { return m_handle; }

    private:
        context& m_context;
        gfx::descriptorgroup_layout_storage m_layout;

        unique_handle<VkDescriptorSetLayout> m_handle;
    };
}

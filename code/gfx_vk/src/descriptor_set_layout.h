#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

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

        vko::Allocator m_allocator{ vko::AllocatorScope::DescriptorSetLayout };
        vko::UniqueHandle<VkDescriptorSetLayout> m_handle;
    };
}

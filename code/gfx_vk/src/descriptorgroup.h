#pragma once

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class descriptorgroup final
    {
    public:
        descriptorgroup(context& context, gfx::descriptorgroup_params const& params);
        ~descriptorgroup();

        VkDescriptorSet get_handle() const { return m_handle; }

    private:
        context& m_context;

        VkDescriptorSet m_handle;
    };
}

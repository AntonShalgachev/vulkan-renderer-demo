#pragma once

#include "gfx/resources.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class descriptorgroup final
    {
    public:
        descriptorgroup(context& context, gfx::descriptorgroup_params const& params);
        ~descriptorgroup();

        VkDescriptorSet get_current_handle() const;

    private:
        context& m_context;

        bool m_is_mutable = false;

        nstl::vector<VkDescriptorSet> m_handles;
    };
}

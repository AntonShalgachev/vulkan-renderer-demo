#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class sampler final
    {
    public:
        sampler(context& context, gfx::sampler_params const& params);
        ~sampler();

        VkSampler get_handle() const { return m_handle; }

    private:
        context& m_context;

        unique_handle<VkSampler> m_handle;
    };
}

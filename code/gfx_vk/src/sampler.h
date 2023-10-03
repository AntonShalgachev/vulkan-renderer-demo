#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class sampler final
    {
    public:
        sampler(context& context, gfx::sampler_params const& params);
        ~sampler();

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::Sampler };
        vko::UniqueHandle<VkSampler> m_handle;
    };
}

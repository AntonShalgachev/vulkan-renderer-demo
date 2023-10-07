#pragma once

#include "gfx_vk/config.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include "nstl/span.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class descriptor_allocator final
    {
    public:
        descriptor_allocator(context& context, descriptors_config const& config);
        ~descriptor_allocator();

        bool allocate(nstl::span<VkDescriptorSetLayout const> layouts, nstl::span<VkDescriptorSet> handles);

        VkDescriptorPool get_handle() const { return m_handle; }

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::DescriptorPool };
        vko::UniqueHandle<VkDescriptorPool> m_handle;
    };
}

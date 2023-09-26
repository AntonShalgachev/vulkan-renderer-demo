#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class buffer final : public gfx::buffer
    {
    public:
        buffer(context& context, gfx::buffer_params const& params);
        ~buffer();

        size_t get_size() const override;
        void upload_sync(nstl::blob_view bytes, size_t offset) override;

    private:
        context& m_context;

        vko::Allocator m_allocator{ vko::AllocatorScope::Buffer };
        vko::UniqueHandle<VkBuffer> m_handle;
    };
}

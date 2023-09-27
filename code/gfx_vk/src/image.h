#pragma once

#include "gfx/resources.h"

#include "vko/Allocator.h"
#include "vko/UniqueHandle.h"

#include "nstl/unique_ptr.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class DeviceMemory;
}

namespace gfx_vk
{
    class context;

    class image final : public gfx::image
    {
    public:
        image(context& context, gfx::image_params const& params);
        ~image();

        void upload_sync(nstl::blob_view bytes) override;

    private:
        context& m_context;

        gfx::image_params m_params;

        vko::Allocator m_allocator{ vko::AllocatorScope::Image };
        vko::UniqueHandle<VkImage> m_handle;

        nstl::unique_ptr<vko::DeviceMemory> m_memory;

        size_t m_memory_size = 0;
    };
}

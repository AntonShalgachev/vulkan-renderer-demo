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

    class buffer final
    {
    public:
        buffer(context& context, gfx::buffer_params const& params);
        ~buffer();

        VkBuffer get_handle() const { return m_handle; }
        size_t get_size() const { return m_params.size; }

        void upload_sync(nstl::blob_view bytes, size_t offset);

    private:
        context& m_context;

        gfx::buffer_params m_params;

        vko::Allocator m_allocator{ vko::AllocatorScope::Buffer };
        vko::UniqueHandle<VkBuffer> m_handle;

        nstl::unique_ptr<vko::DeviceMemory> m_memory;
    };
}

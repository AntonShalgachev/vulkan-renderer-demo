#pragma once

#include "memory.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class buffer final
    {
    public:
        buffer(context& context, gfx::buffer_params const& params);
        ~buffer();

        VkBuffer get_handle(size_t index) const;
        VkBuffer get_current_handle() const;

        bool is_mutable() const { return m_params.is_mutable; }
        size_t get_size() const { return m_params.size; }

        void upload_sync(nstl::blob_view bytes, size_t offset);

    private:
        context& m_context;

        gfx::buffer_params m_params;
        size_t m_aligned_size = 0;

        allocation_handle m_allocation;

        struct impl;
        nstl::vector<impl> m_buffers;
    };
}

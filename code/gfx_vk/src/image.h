#pragma once

#include "memory.h"
#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class image final
    {
    public:
        image(context& context, gfx::image_params const& params, VkImage handle = VK_NULL_HANDLE);
        ~image();

        VkImage get_handle() const { return m_handle; }
        VkImageView get_view_handle() const { return m_view_handle; }

        size_t get_width() const { return m_params.width; }
        size_t get_height() const { return m_params.height; }
        gfx::image_type get_type() const { return m_params.type; }

        void upload_sync(gfx::data_reader& reader);

    private:
        context& m_context;

        gfx::image_params m_params;

        bool m_owns_image = false;
        unique_handle<VkImage> m_handle;
        unique_handle<VkImageView> m_view_handle;

        allocation_handle m_allocation;

        size_t m_memory_size = 0;
    };
}

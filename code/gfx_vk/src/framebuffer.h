#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class framebuffer final
    {
    public:
        framebuffer(context& context, gfx::framebuffer_params const& params);
        ~framebuffer();

        VkFramebuffer get_handle() const { return m_handle; }

        VkExtent2D get_extent() const { return m_extent; }
        nstl::span<gfx::image_type const> get_attachment_types() const { return m_attachment_types; }

    private:
        context& m_context;

        VkExtent2D m_extent{};
        nstl::vector<gfx::image_type> m_attachment_types;

        unique_handle<VkFramebuffer> m_handle;
    };
}

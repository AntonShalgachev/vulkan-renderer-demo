#pragma once

#include "utils.h"

#include "gfx/resources.h"

#include "nstl/unique_ptr.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    enum class color_space
    {
        srgb,
    };

    struct surface_format
    {
        gfx::image_format format = gfx::image_format::r8g8b8;
        color_space color_space = color_space::srgb;
    };

    class swapchain
    {
    public:
        swapchain(context& context, gfx::renderpass_handle renderpass, surface_format surface_format, gfx::image_format depth_format, size_t w, size_t h);
        ~swapchain();

        VkSwapchainKHR get_handle() const;
        VkExtent2D get_extent() const;

        void resize(size_t w, size_t h);

        nstl::span<gfx::framebuffer_handle const> get_framebuffers() const { return m_framebuffers; }

    private:
        void create(size_t w, size_t h);
        void destroy();

    private:
        context& m_context;
        gfx::renderpass_handle m_renderpass;

        surface_format m_surface_format;
        gfx::image_format m_depth_format = gfx::image_format::r8g8b8;
        VkExtent2D m_extent{};

        unique_handle<VkSwapchainKHR> m_handle;
        nstl::vector<VkImage> m_images;

        gfx::image_handle m_depth_image;
        nstl::vector<gfx::image_handle> m_swapchain_images;
        nstl::vector<gfx::framebuffer_handle> m_framebuffers;
    };
}

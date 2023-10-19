#pragma once

#include "gfx/resources.h"

#include "nstl/unique_ptr.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class DeviceMemory;
    class Framebuffer;
    class Image;
    class ImageView;
    class RenderPass;
    class Swapchain;
}

namespace gfx_vk
{
    class context;

    enum class color_space
    {
        srgb,
    };

    struct surface_format
    {
        gfx::image_format format;
        color_space color_space;
    };

    class swapchain
    {
    public:
        swapchain(context& context, gfx::renderpass_handle renderpass, surface_format surface_format, gfx::image_format depth_format, tglm::ivec2 extent);
        ~swapchain();

        VkSwapchainKHR get_handle() const;
        VkExtent2D get_extent() const;

        void resize(tglm::ivec2 extent);

        nstl::span<gfx::framebuffer_handle const> get_framebuffers() const { return m_framebuffers; }

    private:
        void create(tglm::ivec2 extent);
        void destroy();
        void recreate(tglm::ivec2 extent);

    private:
        context& m_context;
        gfx::renderpass_handle m_renderpass;

        surface_format m_surface_format;
        gfx::image_format m_depth_format;

        nstl::unique_ptr<vko::Swapchain> m_swapchain;

        gfx::image_handle m_depth_image;
        nstl::vector<gfx::image_handle> m_swapchain_images;
        nstl::vector<gfx::framebuffer_handle> m_framebuffers;
    };
}

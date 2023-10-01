#pragma once

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
    class Window;
}

namespace gfx_vk
{
    class context;

    class swapchain
    {
    public:
        swapchain(context& context, vko::Window& window, VkRenderPass render_pass, VkSurfaceFormatKHR surface_format, VkFormat depth_format);
        ~swapchain();

    private:
        void on_window_resized();

        void create();
        void destroy();
        void recreate();

    private:
        context& m_context;
        vko::Window& m_window;
        VkRenderPass m_renderpass;

        VkSurfaceFormatKHR m_surface_format;
        VkFormat m_depth_format;

        nstl::unique_ptr<vko::Swapchain> m_swapchain;

        nstl::vector<nstl::unique_ptr<vko::Framebuffer>> m_framebuffers;
        nstl::vector<nstl::unique_ptr<vko::ImageView>> m_image_views;

        nstl::unique_ptr<vko::Image> m_depth_image;
        nstl::unique_ptr<vko::DeviceMemory> m_depth_image_memory;
        nstl::unique_ptr<vko::ImageView> m_depth_image_view;
    };
}

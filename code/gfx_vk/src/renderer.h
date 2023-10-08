#pragma once

#include "swapchain.h"

#include "gfx_vk/config.h"

#include "gfx/resources.h"

#include "nstl/vector.h"

namespace vko
{
    class Window;
}

namespace gfx_vk
{
    class context;
    class swapchain;

    class renderer
    {
    public:
        renderer(context& context, vko::Window& window, renderer_config const& config);
        ~renderer();

        [[nodiscard]] gfx::renderpass_handle get_main_renderpass() const { return m_renderpass; }
        [[nodiscard]] gfx::framebuffer_handle acquire_main_framebuffer();
        [[nodiscard]] float get_main_framebuffer_aspect() const;

        void wait_for_next_frame();
        void begin_frame();

        void renderpass_begin(gfx::renderpass_begin_params const& params);
        void renderpass_end();

        void draw_indexed(gfx::draw_indexed_args const& args);

        void submit();

    private:
        void create_swapchain(vko::Window& window);
        void create_frame_resources(renderer_config const& config);

    private:
        context& m_context;

        gfx::renderpass_handle m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;

        // Temporary images that mimic swapchain images during the transition to the new API
        nstl::vector<gfx::image_handle> m_fake_color_images;
        gfx::image_handle m_fake_depth_image;
        nstl::vector<gfx::framebuffer_handle> m_fake_framebuffers;

        struct frame_resources;
        nstl::vector<frame_resources> m_frame_resources;

        bool m_in_frame = false;
        size_t m_swapchain_image_index = 0;
        size_t m_resources_index = 0;
        size_t m_next_resources_index = 0;

        gfx::renderpass_handle m_current_renderpass = nullptr;
        gfx::framebuffer_handle m_current_framebuffer = nullptr;
    };
}

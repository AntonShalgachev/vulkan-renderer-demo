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

        void begin_resource_update();
        void begin_frame();

        void renderpass_begin(gfx::renderpass_begin_params const& params);
        void renderpass_end();

        void draw_indexed(gfx::draw_indexed_args const& args);

        void submit();

    private:
        struct frame_resources;

    private:
        void create_swapchain(vko::Window& window);
        void create_frame_resources(renderer_config const& config);

        frame_resources& get_current_frame_resources();

    private:
        context& m_context;

        gfx::renderpass_handle m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;

        nstl::vector<frame_resources> m_frame_resources;

        uint32_t m_swapchain_image_index = 0;

        gfx::renderpass_handle m_current_renderpass = nullptr;
        gfx::framebuffer_handle m_current_framebuffer = nullptr;

        // Debug state tracking
        // TODO move to gfx::renderer
        bool m_in_frame = false;
    };
}

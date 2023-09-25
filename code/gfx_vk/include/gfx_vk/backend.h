#pragma once

#include "gfx/backend.h"

namespace vko
{
    class RenderPass;
    class Window;
}

namespace gfx_vk
{
    class context;
    class swapchain;

    class backend final : public gfx::backend
    {
    public:
        backend(vko::Window& window, char const* name, bool enable_validation);
        ~backend();

        nstl::unique_ptr<gfx::buffer> create_buffer() override { return nullptr; }
        nstl::unique_ptr<gfx::image> create_image() override { return nullptr; }
        nstl::unique_ptr<gfx::sampler> create_sampler() override { return nullptr; }
        nstl::unique_ptr<gfx::texture> create_texture() override { return nullptr; }
        nstl::unique_ptr<gfx::framebuffer> create_framebuffer() override { return nullptr; }
        nstl::unique_ptr<gfx::uniforms> create_uniforms() override { return nullptr; }
        nstl::unique_ptr<gfx::shader> create_shader() override { return nullptr; }
        nstl::unique_ptr<gfx::renderstate> create_renderstate() override { return nullptr; }
        nstl::unique_ptr<gfx::renderpass> create_renderpass() override { return nullptr; }

    private:
        nstl::unique_ptr<context> m_context;
        nstl::unique_ptr<vko::RenderPass> m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;
    };
}

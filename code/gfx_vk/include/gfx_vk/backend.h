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

        [[nodiscard]] nstl::unique_ptr<gfx::buffer> create_buffer(gfx::buffer_params const& params) override;
        [[nodiscard]] nstl::unique_ptr<gfx::image> create_image(gfx::image_params const& params) override;
        [[nodiscard]] nstl::unique_ptr<gfx::sampler> create_sampler(gfx::sampler_params const& params) override;
        [[nodiscard]] nstl::unique_ptr<gfx::framebuffer> create_framebuffer() override { return nullptr; }
        [[nodiscard]] nstl::unique_ptr<gfx::uniforms> create_uniforms() override { return nullptr; }
        [[nodiscard]] nstl::unique_ptr<gfx::shader> create_shader(gfx::shader_params const& params) override;
        [[nodiscard]] nstl::unique_ptr<gfx::renderstate> create_renderstate() override { return nullptr; }
        [[nodiscard]] nstl::unique_ptr<gfx::renderpass> create_renderpass() override { return nullptr; }

    private:
        nstl::unique_ptr<context> m_context;
        nstl::unique_ptr<vko::RenderPass> m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;
    };
}

#pragma once

#include "gfx_vk/config.h"

#include "gfx/backend.h"

#include "nstl/vector.h"

// TODO restructure backend, context and other global objects

namespace vko
{
    class Window;
}

namespace gfx_vk
{
    class context;

    class backend final : public gfx::backend
    {
    public:
        backend(vko::Window& window, config const& config);
        backend(vko::Window& window, char const* name, bool enable_validation);
        ~backend();

        [[nodiscard]] gfx::buffer_handle create_buffer(gfx::buffer_params const& params) override;
        void buffer_upload_sync(gfx::buffer_handle handle, nstl::blob_view bytes, size_t offset) override;

        [[nodiscard]] gfx::image_handle create_image(gfx::image_params const& params) override;
        void image_upload_sync(gfx::image_handle handle, nstl::blob_view bytes) override;

        [[nodiscard]] gfx::sampler_handle create_sampler(gfx::sampler_params const& params) override;

        [[nodiscard]] gfx::renderpass_handle create_renderpass(gfx::renderpass_params const& params) override;

        [[nodiscard]] gfx::framebuffer_handle create_framebuffer(gfx::framebuffer_params const& params) override;

        [[nodiscard]] gfx::descriptorgroup_handle create_descriptorgroup(gfx::descriptorgroup_params const& params) override;

        [[nodiscard]] gfx::shader_handle create_shader(gfx::shader_params const& params) override;

        [[nodiscard]] gfx::renderstate_handle create_renderstate(gfx::renderstate_params const& params) override;

        [[nodiscard]] gfx::renderpass_handle get_main_renderpass() override;
        [[nodiscard]] gfx::framebuffer_handle acquire_main_framebuffer() override;
        [[nodiscard]] float get_main_framebuffer_aspect() override;

        void wait_for_next_frame() override;
        void begin_frame() override;

        void renderpass_begin(gfx::renderpass_begin_params const& params) override;
        void renderpass_end() override;

        void draw_indexed(gfx::draw_indexed_args const& args) override;

        void submit() override;

    private:
        nstl::unique_ptr<context> m_context;
    };
}

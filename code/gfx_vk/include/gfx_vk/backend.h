#pragma once

#include "gfx/backend.h"

#include "nstl/vector.h"

namespace vko
{
    class RenderPass;
    class Window;
}

namespace gfx_vk
{
    class context;
    class swapchain;
    class renderpass;
    class descriptor_set_layout;
    class pipeline_layout;

    class backend final : public gfx::backend
    {
    public:
        backend(vko::Window& window, char const* name, bool enable_validation);
        ~backend();

        [[nodiscard]] gfx::buffer* create_buffer(gfx::buffer_params const& params) override;
        void buffer_upload_sync(gfx::buffer* handle, nstl::blob_view bytes, size_t offset) override;

        [[nodiscard]] gfx::image* create_image(gfx::image_params const& params) override;
        void image_upload_sync(gfx::image* handle, nstl::blob_view bytes) override;

        [[nodiscard]] gfx::sampler* create_sampler(gfx::sampler_params const& params) override;

        [[nodiscard]] gfx::renderpass* create_renderpass(gfx::renderpass_params const& params) override;

        [[nodiscard]] gfx::framebuffer* create_framebuffer(gfx::framebuffer_params const& params) override;

        [[nodiscard]] gfx::uniforms* create_uniforms() override { return nullptr; }

        [[nodiscard]] gfx::shader* create_shader(gfx::shader_params const& params) override;

        [[nodiscard]] gfx::renderstate* create_renderstate(gfx::renderstate_params const& params) override;

        [[nodiscard]] gfx::renderpass* get_main_renderpass() override;

    private:
        nstl::unique_ptr<context> m_context;
        nstl::unique_ptr<renderpass> m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;

        // Temporary images that mimic swapchain images during the transition to the new API
        nstl::vector<gfx::image*> m_fake_color_images;
        gfx::image* m_fake_depth_image;
        nstl::vector<gfx::framebuffer*> m_fake_framebuffers;

        nstl::vector<nstl::unique_ptr<descriptor_set_layout>> m_descriptor_set_layouts;
        nstl::vector<nstl::unique_ptr<pipeline_layout>> m_pipeline_layouts;
    };
}

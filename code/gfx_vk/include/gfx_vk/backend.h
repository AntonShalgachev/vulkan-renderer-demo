#pragma once

#include "gfx_vk/config.h"

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

        [[nodiscard]] gfx::renderpass_handle get_main_renderpass() override { return m_renderpass; }
        [[nodiscard]] float get_main_framebuffer_aspect() override;

    private:
        nstl::unique_ptr<context> m_context;
        gfx::renderpass_handle m_renderpass;
        nstl::unique_ptr<swapchain> m_swapchain;

        // Temporary images that mimic swapchain images during the transition to the new API
        nstl::vector<gfx::image_handle> m_fake_color_images;
        gfx::image_handle m_fake_depth_image;
        nstl::vector<gfx::framebuffer_handle> m_fake_framebuffers;
    };
}

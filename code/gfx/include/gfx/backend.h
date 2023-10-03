#pragma once

#include "resources.h"

#include "nstl/blob_view.h"
#include "nstl/unique_ptr.h"

namespace gfx
{
    class backend
    {
    public:
        virtual ~backend() = default;

        [[nodiscard]] virtual buffer_handle create_buffer(buffer_params const& params) = 0;
        virtual void buffer_upload_sync(buffer_handle handle, nstl::blob_view bytes, size_t offset) = 0;

        [[nodiscard]] virtual image_handle create_image(image_params const& params) = 0;
        virtual void image_upload_sync(gfx::image_handle handle, nstl::blob_view bytes) = 0;

        [[nodiscard]] virtual sampler_handle create_sampler(sampler_params const& params) = 0;

        [[nodiscard]] virtual renderpass_handle create_renderpass(renderpass_params const& params) = 0;

        [[nodiscard]] virtual framebuffer_handle create_framebuffer(framebuffer_params const& params) = 0;

        [[nodiscard]] virtual uniforms_handle create_uniforms() = 0;

        [[nodiscard]] virtual shader_handle create_shader(shader_params const& params) = 0;

        [[nodiscard]] virtual renderstate_handle create_renderstate(renderstate_params const& params) = 0;

        [[nodiscard]] virtual renderpass_handle get_main_renderpass() = 0;
    };
}

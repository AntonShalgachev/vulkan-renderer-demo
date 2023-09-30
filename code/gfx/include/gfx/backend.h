#pragma once

#include "nstl/unique_ptr.h"

namespace gfx
{
    struct buffer_params;
    class buffer;
    struct image_params;
    class image;
    struct sampler_params;
    class sampler;
    struct renderpass_params;
    class renderpass;
    class framebuffer;
    class uniforms;
    struct shader_params;
    class shader;
    struct renderstate_params;
    class renderstate;

    class backend
    {
    public:
        virtual ~backend() = default;

        [[nodiscard]] virtual nstl::unique_ptr<buffer> create_buffer(buffer_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<image> create_image(image_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<sampler> create_sampler(sampler_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<renderpass> create_renderpass(renderpass_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<framebuffer> create_framebuffer() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<uniforms> create_uniforms() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<shader> create_shader(shader_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<renderstate> create_renderstate(renderstate_params const& params) = 0;

        [[nodiscard]] virtual renderpass* get_main_renderpass() = 0;
    };
}

#pragma once

#include "nstl/unique_ptr.h"

namespace gfx
{
    struct buffer_params;
    class buffer;
    struct image_params;
    class image;
    class sampler;
    class texture;
    class framebuffer;
    class uniforms;
    class shader;
    class renderstate;
    class renderpass;

    class backend
    {
    public:
        virtual ~backend() = default;

        [[nodiscard]] virtual nstl::unique_ptr<buffer> create_buffer(buffer_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<image> create_image(image_params const& params) = 0;
        [[nodiscard]] virtual nstl::unique_ptr<sampler> create_sampler() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<texture> create_texture() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<framebuffer> create_framebuffer() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<uniforms> create_uniforms() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<shader> create_shader() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<renderstate> create_renderstate() = 0;
        [[nodiscard]] virtual nstl::unique_ptr<renderpass> create_renderpass() = 0;

    private:
    };
}

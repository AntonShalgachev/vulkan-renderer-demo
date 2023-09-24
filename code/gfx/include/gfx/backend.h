#pragma once

#include "nstl/unique_ptr.h"

namespace gfx
{
    class buffer;
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
        virtual nstl::unique_ptr<buffer> create_buffer() = 0;
        virtual nstl::unique_ptr<image> create_image() = 0;
        virtual nstl::unique_ptr<sampler> create_sampler() = 0;
        virtual nstl::unique_ptr<texture> create_texture() = 0;
        virtual nstl::unique_ptr<framebuffer> create_framebuffer() = 0;
        virtual nstl::unique_ptr<uniforms> create_uniforms() = 0;
        virtual nstl::unique_ptr<shader> create_shader() = 0;
        virtual nstl::unique_ptr<renderstate> create_renderstate() = 0;
        virtual nstl::unique_ptr<renderpass> create_renderpass() = 0;

    private:
    };
}

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
    struct framebuffer_params;
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

        [[nodiscard]] virtual buffer* create_buffer(buffer_params const& params) = 0;
        [[nodiscard]] virtual image* create_image(image_params const& params) = 0;
        [[nodiscard]] virtual sampler* create_sampler(sampler_params const& params) = 0;
        [[nodiscard]] virtual renderpass* create_renderpass(renderpass_params const& params) = 0;
        [[nodiscard]] virtual framebuffer* create_framebuffer(framebuffer_params const& params) = 0;
        [[nodiscard]] virtual uniforms* create_uniforms() = 0;
        [[nodiscard]] virtual shader* create_shader(shader_params const& params) = 0;
        [[nodiscard]] virtual renderstate* create_renderstate(renderstate_params const& params) = 0;

        [[nodiscard]] virtual renderpass* get_main_renderpass() = 0;
    };
}

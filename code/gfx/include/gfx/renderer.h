#pragma once

#include "gfx/backend.h"
#include "gfx/resources.h"

#include "nstl/span.h"
#include "nstl/unique_ptr.h"

#include <stdint.h>

namespace gfx
{
    class buffer;
    class image;
    class sampler;
    class renderpass;
    class framebuffer;
    class uniforms;
    class shader;
    class renderstate;

    class renderer
    {
    public:
        renderer(nstl::unique_ptr<backend> backend);
        void set_backend(nstl::unique_ptr<backend> backend);

        // TODO add some basic validation before calling backend

        [[nodiscard]] nstl::unique_ptr<buffer> create_buffer(buffer_params const& params) { return m_backend->create_buffer(params); }
        [[nodiscard]] nstl::unique_ptr<image> create_image(image_params const& params) { return m_backend->create_image(params); }
        [[nodiscard]] nstl::unique_ptr<sampler> create_sampler(sampler_params const& params) { return m_backend->create_sampler(params); }
        [[nodiscard]] nstl::unique_ptr<renderpass> create_renderpass(renderpass_params const& params) { return m_backend->create_renderpass(params); }
        [[nodiscard]] nstl::unique_ptr<framebuffer> create_framebuffer() { return m_backend->create_framebuffer(); }
        [[nodiscard]] nstl::unique_ptr<uniforms> create_uniforms() { return m_backend->create_uniforms(); }
        [[nodiscard]] nstl::unique_ptr<shader> create_shader(shader_params const& params) { return m_backend->create_shader(params); }
        [[nodiscard]] nstl::unique_ptr<renderstate> create_renderstate(renderstate_params const& params) { return m_backend->create_renderstate(params); }
        // TODO add API for sharing renderstates (i.e. to avoid creating renderstates with the same parameters)

        [[nodiscard]] renderpass* get_main_renderpass() { return m_backend->get_main_renderpass(); }

        void begin_renderpass(renderpass& renderpass);
        void end_renderpass(renderpass& renderpass);

        void set_renderstate(renderstate& renderstate);
        void set_uniforms(uint8_t slot_index, uniforms& uniforms);
        void set_index_buffer(buffer& buffer, size_t offset);
        void set_vertex_buffers(nstl::span<buffer const> buffers, nstl::span<size_t const> offsets);
        void set_scissor();

        void draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t vertex_offset);

    private:
        nstl::unique_ptr<backend> m_backend;
    };
}

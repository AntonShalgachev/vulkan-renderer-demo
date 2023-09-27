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
    class texture;
    class framebuffer;
    class uniforms;
    class shader;
    class renderstate;
    class renderpass;

    class renderer
    {
    public:
        renderer(nstl::unique_ptr<backend> backend);
        void set_backend(nstl::unique_ptr<backend> backend);

        [[nodiscard]] nstl::unique_ptr<buffer> create_buffer(buffer_params const& params) { return m_backend->create_buffer(params); }
        [[nodiscard]] nstl::unique_ptr<image> create_image(image_params const& params) { return m_backend->create_image(params); }
        [[nodiscard]] nstl::unique_ptr<sampler> create_sampler() { return m_backend->create_sampler(); }
        [[nodiscard]] nstl::unique_ptr<texture> create_texture() { return m_backend->create_texture(); }
        [[nodiscard]] nstl::unique_ptr<framebuffer> create_framebuffer() { return m_backend->create_framebuffer(); }
        [[nodiscard]] nstl::unique_ptr<uniforms> create_uniforms() { return m_backend->create_uniforms(); }
        [[nodiscard]] nstl::unique_ptr<shader> create_shader() { return m_backend->create_shader(); }
        [[nodiscard]] nstl::unique_ptr<renderstate> create_renderstate() { return m_backend->create_renderstate(); }
        [[nodiscard]] nstl::unique_ptr<renderpass> create_renderpass() { return m_backend->create_renderpass(); }

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

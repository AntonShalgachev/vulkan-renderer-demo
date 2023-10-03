#pragma once

#include "gfx/resources.h"

#include "nstl/unique_ptr.h"
#include "nstl/vector.h"

namespace gfx_vk
{
    class context;

    class buffer;
    class image;
    class sampler;
    class renderpass;
    class framebuffer;
    class shader;
    class renderstate;

    struct renderstate_init_params;

    class resource_container final
    {
    public:
        resource_container(context& context);
        ~resource_container();

        [[nodiscard]] gfx::buffer_handle create_buffer(gfx::buffer_params const& params);
        [[nodiscard]] buffer& get_buffer(gfx::buffer_handle handle) const;
        [[nodiscard]] bool destroy_buffer(gfx::buffer_handle handle);

        gfx::image* create_image(gfx::image_params const& params);

        gfx::sampler* create_sampler(gfx::sampler_params const& params);

        gfx::renderpass* create_renderpass(gfx::renderpass_params const& params);

        gfx::framebuffer* create_framebuffer(gfx::framebuffer_params const& params);

        gfx::shader* create_shader(gfx::shader_params const& params);

        gfx::renderstate* create_renderstate(renderstate_init_params const& params);

    private:
        context& m_context;

        nstl::vector<nstl::unique_ptr<buffer>> m_buffers;
        nstl::vector<nstl::unique_ptr<image>> m_images;
        nstl::vector<nstl::unique_ptr<sampler>> m_samplers;
        nstl::vector<nstl::unique_ptr<renderpass>> m_renderpasses;
        nstl::vector<nstl::unique_ptr<framebuffer>> m_framebuffers;
        nstl::vector<nstl::unique_ptr<shader>> m_shaders;
        nstl::vector<nstl::unique_ptr<renderstate>> m_renderstates;
    };
}

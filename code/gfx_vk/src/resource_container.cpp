#include "resource_container.h"

#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "shader.h"
#include "renderstate.h"

namespace
{
    template<typename T, typename P>
    T* create_resource(gfx_vk::context& context, nstl::vector<nstl::unique_ptr<T>>& container, P const& params)
    {
        container.push_back(nstl::make_unique<T>(context, params));
        return container.back().get();
    }
}

gfx_vk::resource_container::resource_container(context& context) : m_context(context) {}

gfx_vk::resource_container::~resource_container() = default;

gfx::buffer* gfx_vk::resource_container::create_buffer(gfx::buffer_params const& params)
{
    return create_resource<buffer>(m_context, buffers, params);
}

gfx::image* gfx_vk::resource_container::create_image(gfx::image_params const& params)
{
    return create_resource<image>(m_context, images, params);
}

gfx::sampler* gfx_vk::resource_container::create_sampler(gfx::sampler_params const& params)
{
    return create_resource<sampler>(m_context, samplers, params);
}

gfx::renderpass* gfx_vk::resource_container::create_renderpass(gfx::renderpass_params const& params)
{
    return create_resource<renderpass>(m_context, renderpasses, params);
}

gfx::framebuffer* gfx_vk::resource_container::create_framebuffer(gfx::framebuffer_params const& params)
{
    return create_resource<framebuffer>(m_context, framebuffers, params);
}

gfx::shader* gfx_vk::resource_container::create_shader(gfx::shader_params const& params)
{
    return create_resource<shader>(m_context, shaders, params);
}

gfx::renderstate* gfx_vk::resource_container::create_renderstate(renderstate_init_params const& params)
{
    return create_resource<renderstate>(m_context, renderstates, params);
}

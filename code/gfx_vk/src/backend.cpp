#include "gfx_vk/backend.h"

#include "context.h"

#include "buffer.h"
#include "image.h"

gfx_vk::backend::backend(vko::Window& window, config const& config)
    : m_context(nstl::make_unique<context>(window, config))
{
    
}

gfx_vk::backend::~backend()
{
    m_context->get_device().waitIdle();
}

gfx::buffer_handle gfx_vk::backend::create_buffer(gfx::buffer_params const& params)
{
    return m_context->get_resources().create_buffer(params);
}

void gfx_vk::backend::buffer_upload_sync(gfx::buffer_handle handle, nstl::blob_view bytes, size_t offset)
{
    return m_context->get_resources().get_buffer(handle).upload_sync(bytes, offset);
}

gfx::image_handle gfx_vk::backend::create_image(gfx::image_params const& params)
{
    return m_context->get_resources().create_image(params);
}

void gfx_vk::backend::image_upload_sync(gfx::image_handle handle, nstl::blob_view bytes)
{
    return m_context->get_resources().get_image(handle).upload_sync(bytes);
}

gfx::sampler_handle gfx_vk::backend::create_sampler(gfx::sampler_params const& params)
{
    return m_context->get_resources().create_sampler(params);
}

gfx::renderpass_handle gfx_vk::backend::create_renderpass(gfx::renderpass_params const& params)
{
    return m_context->get_resources().create_renderpass(params);
}

gfx::framebuffer_handle gfx_vk::backend::create_framebuffer(gfx::framebuffer_params const& params)
{
    return m_context->get_resources().create_framebuffer(params);
}

gfx::descriptorgroup_handle gfx_vk::backend::create_descriptorgroup(gfx::descriptorgroup_params const& params)
{
    return m_context->get_resources().create_descriptorgroup(params);
}

gfx::shader_handle gfx_vk::backend::create_shader(gfx::shader_params const& params)
{
    return m_context->get_resources().create_shader(params);
}

gfx::renderstate_handle gfx_vk::backend::create_renderstate(gfx::renderstate_params const& params)
{
    return m_context->get_resources().create_renderstate(params);
}

gfx::renderpass_handle gfx_vk::backend::get_main_renderpass()
{
    return m_context->get_renderer().get_main_renderpass();
}

gfx::framebuffer_handle gfx_vk::backend::acquire_main_framebuffer()
{
    return m_context->get_renderer().acquire_main_framebuffer();
}

float gfx_vk::backend::get_main_framebuffer_aspect()
{
    return m_context->get_renderer().get_main_framebuffer_aspect();
}

void gfx_vk::backend::wait_for_next_frame()
{
    return m_context->get_renderer().wait_for_next_frame();
}

void gfx_vk::backend::begin_frame()
{
    return m_context->get_renderer().begin_frame();
}

void gfx_vk::backend::renderpass_begin(gfx::renderpass_begin_params const& params)
{
    return m_context->get_renderer().renderpass_begin(params);
}

void gfx_vk::backend::renderpass_end()
{
    return m_context->get_renderer().renderpass_end();
}

void gfx_vk::backend::draw_indexed(gfx::draw_indexed_args const& args)
{
    return m_context->get_renderer().draw_indexed(args);
}

void gfx_vk::backend::submit()
{
    return m_context->get_renderer().submit();
}

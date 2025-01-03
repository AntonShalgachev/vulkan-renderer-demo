#include "resource_container.h"

#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "descriptorgroup.h"
#include "shader.h"
#include "descriptor_set_layout.h"
#include "pipeline_layout.h"
#include "renderstate.h"

namespace
{
    template<typename T, typename H>
    H create_handle(T& resource)
    {
        return { &resource };
    }

    template<typename T, typename H, typename... Args>
    H create_resource(nstl::vector<nstl::unique_ptr<T>>& container, gfx_vk::context& context, Args&&... params)
    {
        container.push_back(nstl::make_unique<T>(context, nstl::forward<Args>(params)...));
        return { container.back().get() };
    }

    template<typename T, typename H>
    T& get_resource(nstl::vector<nstl::unique_ptr<T>> const&, H handle)
    {
        assert(handle);
        return *static_cast<T*>(handle.ptr);
    }

    template<typename T, typename H>
    bool destroy_resource(nstl::vector<nstl::unique_ptr<T>>& container, H handle)
    {
        for (auto& resource : container)
        {
            if (resource.get() == static_cast<T*>(handle.ptr))
            {
                resource = nullptr;
                return true;
            }
        }

        return false;
    }
}

gfx_vk::resource_container::resource_container(context& context) : m_context(context) {}

gfx_vk::resource_container::~resource_container() = default;

gfx::buffer_handle gfx_vk::resource_container::create_buffer(gfx::buffer_params const& params)
{
    return create_resource<buffer, gfx::buffer_handle>(m_buffers, m_context, params);
}

gfx_vk::buffer& gfx_vk::resource_container::get_buffer(gfx::buffer_handle handle) const
{
    return get_resource(m_buffers, handle);
}

bool gfx_vk::resource_container::destroy_buffer(gfx::buffer_handle handle)
{
    return destroy_resource(m_buffers, handle);
}

gfx::image_handle gfx_vk::resource_container::create_image(gfx::image_params const& params, VkImage handle)
{
    return create_resource<image, gfx::image_handle>(m_images, m_context, params, handle);
}

gfx_vk::image& gfx_vk::resource_container::get_image(gfx::image_handle handle) const
{
    return get_resource(m_images, handle);
}

bool gfx_vk::resource_container::destroy_image(gfx::image_handle handle)
{
    return destroy_resource(m_images, handle);
}

gfx::sampler_handle gfx_vk::resource_container::create_sampler(gfx::sampler_params const& params)
{
    // TODO reuse already existing samplers
    return create_resource<sampler, gfx::sampler_handle>(m_samplers, m_context, params);
}

gfx_vk::sampler& gfx_vk::resource_container::get_sampler(gfx::sampler_handle handle) const
{
    return get_resource(m_samplers, handle);
}

bool gfx_vk::resource_container::destroy_sampler(gfx::sampler_handle handle)
{
    return destroy_resource(m_samplers, handle);
}

gfx::renderpass_handle gfx_vk::resource_container::create_renderpass(gfx::renderpass_params const& params)
{
    return create_resource<renderpass, gfx::renderpass_handle>(m_renderpasses, m_context, params);
}

gfx_vk::renderpass& gfx_vk::resource_container::get_renderpass(gfx::renderpass_handle handle) const
{
    return get_resource(m_renderpasses, handle);
}

bool gfx_vk::resource_container::destroy_renderpass(gfx::renderpass_handle handle)
{
    return destroy_resource(m_renderpasses, handle);
}

gfx::framebuffer_handle gfx_vk::resource_container::create_framebuffer(gfx::framebuffer_params const& params)
{
    return create_resource<framebuffer, gfx::framebuffer_handle>(m_framebuffers, m_context, params);
}

gfx_vk::framebuffer& gfx_vk::resource_container::get_framebuffer(gfx::framebuffer_handle handle) const
{
    return get_resource(m_framebuffers, handle);
}

bool gfx_vk::resource_container::destroy_framebuffer(gfx::framebuffer_handle handle)
{
    return destroy_resource(m_framebuffers, handle);
}

gfx::descriptorgroup_handle gfx_vk::resource_container::create_descriptorgroup(gfx::descriptorgroup_params const& params)
{
    return create_resource<descriptorgroup, gfx::descriptorgroup_handle>(m_descriptorgroups, m_context, params);
}

gfx_vk::descriptorgroup& gfx_vk::resource_container::get_descriptorgroup(gfx::descriptorgroup_handle handle) const
{
    return get_resource(m_descriptorgroups, handle);
}

bool gfx_vk::resource_container::destroy_descriptorgroup(gfx::descriptorgroup_handle handle)
{
    return destroy_resource(m_descriptorgroups, handle);
}

gfx::shader_handle gfx_vk::resource_container::create_shader(gfx::shader_params const& params)
{
    return create_resource<shader, gfx::shader_handle>(m_shaders, m_context, params);
}

gfx_vk::shader& gfx_vk::resource_container::get_shader(gfx::shader_handle handle) const
{
    return get_resource(m_shaders, handle);
}

bool gfx_vk::resource_container::destroy_shader(gfx::shader_handle handle)
{
    return destroy_resource(m_shaders, handle);
}

gfx::renderstate_handle gfx_vk::resource_container::create_renderstate(gfx::renderstate_params const& params)
{
    nstl::vector<VkDescriptorSetLayout> set_layout_handles;

    for (gfx::descriptorgroup_layout_view const& layout : params.descriptorgroup_layouts)
    {
        set_layout_handles.push_back(create_descriptor_set_layout(layout));
    }

    renderstate_init_params init_params = {
        .shaders = params.shaders,
        .vertex_config = params.vertex_config,
        .flags = params.flags,

        .layout = create_pipeline_layout(set_layout_handles),
        .renderpass = get_renderpass(params.renderpass).get_handle(),
    };

    for (auto const& state : m_renderstates)
    {
        if (state->get_params() == init_params)
        {
            // TODO increase ref counter
            return create_handle<renderstate, gfx::renderstate_handle>(*state.get());
        }
    }

    return create_resource<renderstate, gfx::renderstate_handle>(m_renderstates, m_context, init_params);
}

gfx_vk::renderstate& gfx_vk::resource_container::get_renderstate(gfx::renderstate_handle handle) const
{
    return get_resource(m_renderstates, handle);
}

bool gfx_vk::resource_container::destroy_renderstate(gfx::renderstate_handle)
{
    // TODO check the ref counter first before destroying the resource
    // TODO also destroy descriptor set layouts and the pipeline layout
//     return destroy_resource(m_renderstates, handle);
    return false;
}

VkDescriptorSetLayout gfx_vk::resource_container::create_descriptor_set_layout(gfx::descriptorgroup_layout_view const& layout)
{
    for (auto const& set_layout : m_descriptor_set_layouts)
    {
        if (set_layout->get_layout() == layout)
        {
            // TODO increase ref counter
            return set_layout->get_handle();
        }
    }

    m_descriptor_set_layouts.push_back(nstl::make_unique<descriptor_set_layout>(m_context, layout));
    return m_descriptor_set_layouts.back()->get_handle();
}

VkPipelineLayout gfx_vk::resource_container::create_pipeline_layout(nstl::span<VkDescriptorSetLayout const> layouts)
{
    for (auto const& layout : m_pipeline_layouts)
    {
        if (layout->get_layouts() == layouts)
        {
            // TODO increase ref counter
            return layout->get_handle();
        }
    }

    m_pipeline_layouts.push_back(nstl::make_unique<pipeline_layout>(m_context, layouts));
    return m_pipeline_layouts.back()->get_handle();
}

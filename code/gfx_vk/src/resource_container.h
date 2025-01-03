#pragma once

#include "gfx/resources.h"

#include "nstl/unique_ptr.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    class buffer;
    class image;
    class sampler;
    class renderpass;
    class framebuffer;
    class descriptorgroup;
    class shader;
    class descriptor_set_layout;
    class pipeline_layout;
    class renderstate;

    struct renderstate_init_params;

    class resource_container final
    {
    public:
        resource_container(context& context);
        ~resource_container();

        // TODO get rid of these functions
        [[nodiscard]] gfx::buffer_handle create_buffer(gfx::buffer_params const& params);
        [[nodiscard]] buffer& get_buffer(gfx::buffer_handle handle) const;
        [[nodiscard]] bool destroy_buffer(gfx::buffer_handle handle);

        [[nodiscard]] gfx::image_handle create_image(gfx::image_params const& params, VkImage handle = VK_NULL_HANDLE);
        [[nodiscard]] image& get_image(gfx::image_handle handle) const;
        [[nodiscard]] bool destroy_image(gfx::image_handle handle);

        [[nodiscard]] gfx::sampler_handle create_sampler(gfx::sampler_params const& params);
        [[nodiscard]] sampler& get_sampler(gfx::sampler_handle handle) const;
        [[nodiscard]] bool destroy_sampler(gfx::sampler_handle handle);

        [[nodiscard]] gfx::renderpass_handle create_renderpass(gfx::renderpass_params const& params);
        [[nodiscard]] renderpass& get_renderpass(gfx::renderpass_handle handle) const;
        [[nodiscard]] bool destroy_renderpass(gfx::renderpass_handle handle);

        [[nodiscard]] gfx::framebuffer_handle create_framebuffer(gfx::framebuffer_params const& params);
        [[nodiscard]] framebuffer& get_framebuffer(gfx::framebuffer_handle handle) const;
        [[nodiscard]] bool destroy_framebuffer(gfx::framebuffer_handle handle);

        [[nodiscard]] gfx::descriptorgroup_handle create_descriptorgroup(gfx::descriptorgroup_params const& params);
        [[nodiscard]] descriptorgroup& get_descriptorgroup(gfx::descriptorgroup_handle handle) const;
        [[nodiscard]] bool destroy_descriptorgroup(gfx::descriptorgroup_handle handle);

        [[nodiscard]] gfx::shader_handle create_shader(gfx::shader_params const& params);
        [[nodiscard]] shader& get_shader(gfx::shader_handle handle) const;
        [[nodiscard]] bool destroy_shader(gfx::shader_handle handle);

        [[nodiscard]] gfx::renderstate_handle create_renderstate(gfx::renderstate_params const& params);
        [[nodiscard]] renderstate& get_renderstate(gfx::renderstate_handle handle) const;
        [[nodiscard]] bool destroy_renderstate(gfx::renderstate_handle handle);

        [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout(gfx::descriptorgroup_layout_view const& layout);
        [[nodiscard]] VkPipelineLayout create_pipeline_layout(nstl::span<VkDescriptorSetLayout const> layouts);

    private:
        context& m_context;

        nstl::vector<nstl::unique_ptr<buffer>> m_buffers;
        nstl::vector<nstl::unique_ptr<image>> m_images;
        nstl::vector<nstl::unique_ptr<sampler>> m_samplers;
        nstl::vector<nstl::unique_ptr<renderpass>> m_renderpasses;
        nstl::vector<nstl::unique_ptr<framebuffer>> m_framebuffers;
        nstl::vector<nstl::unique_ptr<descriptorgroup>> m_descriptorgroups;
        nstl::vector<nstl::unique_ptr<shader>> m_shaders;

        nstl::vector<nstl::unique_ptr<descriptor_set_layout>> m_descriptor_set_layouts;
        nstl::vector<nstl::unique_ptr<pipeline_layout>> m_pipeline_layouts;
        nstl::vector<nstl::unique_ptr<renderstate>> m_renderstates;
    };
}

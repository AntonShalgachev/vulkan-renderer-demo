#pragma once

#include "nstl/blob_view.h"
#include "nstl/span.h"
#include "nstl/optional.h"

namespace gfx
{
    // TODO make resources opaque handles

    struct handle
    {
        handle() = default;
        handle(nullptr_t) {}
        handle(void* ptr) : ptr(ptr) {}
        explicit operator bool() const { return ptr != nullptr; }

        void* ptr = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class buffer_usage
    {
        vertex_index,
        uniform,
    };

    enum class buffer_location
    {
        device_local,
        host_visible,
    };

    struct buffer_params
    {
        size_t size = 0;
        buffer_usage usage = buffer_usage::vertex_index;
        buffer_location location = buffer_location::device_local;
        bool is_mutable = false;
    };

    struct buffer_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class image_format
    {
        r8g8b8a8,
        b8g8r8a8_srgb,
        r8g8b8,
        bc1_unorm,
        bc3_unorm,
        bc5_unorm,
        d32_float,
    };

    enum class image_type
    {
        color,
        depth,
    };

    // TODO change to bit flags
    enum class image_usage
    {
        color,
        depth_sampled,
        depth,
        upload_sampled,
    };

    struct image_params
    {
        size_t width = 0;
        size_t height = 0;
        image_format format = image_format::r8g8b8a8;
        image_type type = image_type::color; // TODO shouldn't belong to the image
        image_usage usage = image_usage::upload_sampled;
    };

    struct image_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class sampler_filter_mode
    {
        nearest,
        linear,
    };

    enum class sampler_wrap_mode
    {
        repeat,
        mirror,
        clamp_to_edge,
    };

    struct sampler_params
    {
        sampler_filter_mode mag_filter = sampler_filter_mode::linear;
        sampler_filter_mode min_filter = sampler_filter_mode::linear;
        sampler_wrap_mode wrap_u = sampler_wrap_mode::repeat;
        sampler_wrap_mode wrap_v = sampler_wrap_mode::repeat;
    };

    struct sampler_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    struct renderpass_params
    {
        nstl::span<image_format> color_attachment_formats;
        nstl::optional<image_format> depth_stencil_attachment_format;

        // TODO rework
        bool has_presentable_images = true;
        bool keep_depth_values_after_renderpass = false;
    };

    struct renderpass_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    struct framebuffer_params
    {
        nstl::span<image_handle const> attachments;
        renderpass_handle renderpass = nullptr;
    };

    struct framebuffer_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    // TODO rename?

    enum class descriptor_type
    {
        buffer,
        combined_image_sampler,
    };

    struct descriptorgroup_ref
    {
        descriptorgroup_ref(buffer_handle buffer) : type(descriptor_type::buffer), buffer(buffer) {}
        descriptorgroup_ref(image_handle image, sampler_handle sampler) : type(descriptor_type::combined_image_sampler), combined_image_sampler({image, sampler}) {}

        descriptor_type type = descriptor_type::buffer;

        union
        {
            buffer_handle buffer;

            struct 
            {
                image_handle image;
                sampler_handle sampler;

            } combined_image_sampler;
        };
    };

    struct descriptorgroup_entry
    {
        size_t location = 0;
        descriptorgroup_ref resource;

    };

    struct descriptorgroup_params
    {
        nstl::span<descriptorgroup_entry const> entries;
    };

    struct descriptorgroup_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class shader_stage
    {
        vertex,
        geometry,
        fragment,
    };

    struct shader_params
    {
        nstl::string_view filename; // TODO: replace with something less backend-dependent
        shader_stage stage = shader_stage::vertex;
        nstl::string_view entry_point = "main";
    };

    struct shader_handle : handle
    {
        using handle::handle;
    };

    //////////////////////////////////////////////////////////////////////////

    enum class attribute_type
    {
        vec2f,
        vec3f,
        vec4f,
        uint32,
        mat2f,
        mat3f,
        mat4f,
    };

    struct attribute_description
    {
        size_t location = 0;
        size_t offset = 0;
        size_t stride = 0;
        attribute_type type = attribute_type::vec4f;
    };

    enum class vertex_topology
    {
        triangles,
        triangle_strip,
        triangle_fan,
    };

    struct vertex_configuration
    {
        nstl::span<attribute_description const> attributes;
        vertex_topology topology = vertex_topology::triangles;
    };

    struct uniform_group_configuration
    {
        // TODO rework
        bool has_buffer = true;
        bool has_albedo_texture = true;
        bool has_normal_map = false;
        bool has_shadow_map = false;
    };

    struct renderstate_flags
    {
        // TODO rework
        // TODO some flags have additional parameters; add ability to specify them
        bool cull_backfaces = true;
        bool wireframe = false;
        bool depth_test = true;
        bool alpha_blending = false;
        bool depth_bias = false;
    };

    struct renderstate_params
    {
        nstl::span<shader_handle const> shaders;
        renderpass_handle renderpass = nullptr;
        vertex_configuration vertex_config;
        nstl::span<uniform_group_configuration> uniform_groups_config;
        renderstate_flags flags;
    };

    struct renderstate_handle : handle
    {
        using handle::handle;
    };
}

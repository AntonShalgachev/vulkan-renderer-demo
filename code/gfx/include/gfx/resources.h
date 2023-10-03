#pragma once

#include "nstl/blob_view.h"
#include "nstl/span.h"
#include "nstl/optional.h"

namespace gfx
{
    // TODO make resources opaque handles

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

    class buffer
    {
    public:
        virtual ~buffer() = default;
    };

    using buffer_handle = buffer*;

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

    class image
    {
    public:
        virtual ~image() = default;
    };

    using image_handle = image*;

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

    class sampler
    {
    public:
        virtual ~sampler() = default;
    };

    using sampler_handle = sampler*;

    //////////////////////////////////////////////////////////////////////////

    struct renderpass_params
    {
        nstl::span<image_format> color_attachment_formats;
        nstl::optional<image_format> depth_stencil_attachment_format;

        // TODO rework
        bool has_presentable_images = true;
        bool keep_depth_values_after_renderpass = false;
    };

    class renderpass
    {
    public:
        virtual ~renderpass() = default;
    };

    using renderpass_handle = renderpass*;

    //////////////////////////////////////////////////////////////////////////

    struct framebuffer_params
    {
        nstl::span<image_handle const> attachments;
        renderpass_handle renderpass = nullptr;
    };

    class framebuffer
    {
    public:
        virtual ~framebuffer() = default;
    };

    using framebuffer_handle = framebuffer*;

    //////////////////////////////////////////////////////////////////////////

    // TODO rename?
    class descriptorgroup
    {
    public:
        virtual ~descriptorgroup() = default;
    };

    using descriptorgroup_handle = descriptorgroup*;

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

    class shader
    {
    public:
        virtual ~shader() = default;
    };

    using shader_handle = shader*;

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

    class renderstate
    {
    public:
        virtual ~renderstate() = default;
    };

    using renderstate_handle = renderstate*;
}

#pragma once

#include "nstl/blob_view.h"

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

        [[nodiscard]] virtual size_t get_size() const = 0;
        virtual void upload_sync(nstl::blob_view bytes, size_t offset = 0) = 0;
        // TODO: add async upload
    };

    //////////////////////////////////////////////////////////////////////////

    enum class image_format
    {
        r8g8b8a8,
        r8g8b8,
        bc1_unorm,
        bc3_unorm,
        bc5_unorm,
    };

    struct image_params
    {
        size_t width = 0;
        size_t height = 0;
        image_format format = image_format::r8g8b8a8;
    };

    class image
    {
    public:
        virtual ~image() = default;

        virtual void upload_sync(nstl::blob_view bytes) = 0;
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

    class sampler
    {
    public:
        virtual ~sampler() = default;
    };

    //////////////////////////////////////////////////////////////////////////

    class framebuffer
    {
    public:
        virtual ~framebuffer() = default;
    };

    class uniforms
    {
    public:
        virtual ~uniforms() = default;
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

    class shader
    {
    public:
        virtual ~shader() = default;
    };

    //////////////////////////////////////////////////////////////////////////

    class renderstate
    {
    public:
        virtual ~renderstate() = default;
    };

    class renderpass
    {
    public:
        virtual ~renderpass() = default;
    };
}

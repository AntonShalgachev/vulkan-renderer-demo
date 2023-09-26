#pragma once

#include "nstl/blob_view.h"

namespace gfx
{
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

    class image
    {
    public:
        virtual ~image() = default;
    };

    class sampler
    {
    public:
        virtual ~sampler() = default;
    };

    class texture
    {
    public:
        virtual ~texture() = default;
    };

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

    class shader
    {
    public:
        virtual ~shader() = default;
    };

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

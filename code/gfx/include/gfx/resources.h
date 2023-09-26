#pragma once

namespace gfx
{
    class buffer
    {
    public:
        virtual ~buffer() = default;
    };

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

#pragma once

namespace vkgfx
{
    struct ImageHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct BufferHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct ShaderModuleHandle
    {
        std::size_t index = 0; // TODO improve

        auto operator<=>(ShaderModuleHandle const&) const = default;
    };

    struct SamplerHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct TextureHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct MaterialHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct MeshHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct DescriptorSetLayoutHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct PipelineLayoutHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct PipelineHandle
    {
        std::size_t index = 0; // TODO improve
    };
}

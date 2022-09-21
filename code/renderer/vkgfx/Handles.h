#pragma once

namespace vkgfx
{
    struct ResourceHandle
    {
        std::size_t index = static_cast<std::size_t>(0); // TODO make it -1 by default

        auto operator<=>(ResourceHandle const&) const = default;
    };

    struct ImageHandle : ResourceHandle
    {
        
    };

    struct BufferHandle : ResourceHandle
    {
        
    };

    struct ShaderModuleHandle : ResourceHandle
    {
        
    };

    struct SamplerHandle : ResourceHandle
    {
        
    };

    struct TextureHandle : ResourceHandle
    {
        
    };

    struct MaterialHandle : ResourceHandle
    {
        
    };

    struct MeshHandle : ResourceHandle
    {
        
    };

    struct DescriptorSetLayoutHandle : ResourceHandle
    {
        
    };

    struct PipelineLayoutHandle : ResourceHandle
    {
        
    };

    struct PipelineHandle : ResourceHandle
    {
        
    };
}

#pragma once

namespace vkgfx
{
    struct ResourceHandle
    {
        std::size_t index = static_cast<std::size_t>(-1);

        auto operator<=>(ResourceHandle const&) const = default;

        operator bool() const
        {
            return *this != ResourceHandle{};
        }
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

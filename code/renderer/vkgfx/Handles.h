#pragma once

#include "ResourceContainer.h"

namespace vkgfx
{
    struct OldResourceHandle
    {
        std::size_t index = static_cast<std::size_t>(-1);

        auto operator<=>(OldResourceHandle const&) const = default;

        operator bool() const
        {
            return *this != OldResourceHandle{};
        }
    };

    struct ImageHandle : OldResourceHandle
    {
        
    };

    struct BufferHandle : ResourceHandle
    {
        
    };

    struct ShaderModuleHandle : OldResourceHandle
    {
        
    };

    struct SamplerHandle : OldResourceHandle
    {
        
    };

    struct TextureHandle : OldResourceHandle
    {
        
    };

    struct MaterialHandle : OldResourceHandle
    {
        
    };

    struct MeshHandle : OldResourceHandle
    {
        
    };

    struct DescriptorSetLayoutHandle : OldResourceHandle
    {
        
    };

    struct PipelineLayoutHandle : OldResourceHandle
    {
        
    };

    struct PipelineHandle : OldResourceHandle
    {
        
    };
}

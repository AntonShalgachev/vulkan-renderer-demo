#pragma once

#include "ResourceContainer.h"

namespace vkgfx
{
    struct OldResourceHandle
    {
        std::size_t index = static_cast<std::size_t>(-1);

        auto operator<=>(OldResourceHandle const&) const = default;

        explicit operator bool() const
        {
            return *this != OldResourceHandle{};
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

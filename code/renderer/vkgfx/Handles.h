#pragma once

#include "renderer/vkgfx/ResourceContainer.h"

#include "nstl/hash.h"

#define DEFINE_RESOURCE_HANDLE_HASH(T) template<> struct nstl::hash<T> { size_t operator()(T const& value) const { return nstl::hash<vkgfx::ResourceHandle>{}(static_cast<vkgfx::ResourceHandle>(value)); } }

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

DEFINE_RESOURCE_HANDLE_HASH(vkgfx::ShaderModuleHandle);

#undef DEFINE_RESOURCE_HANDLE_HASH

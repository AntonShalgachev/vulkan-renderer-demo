#pragma once

#include "vkgfx/Handles.h"

#include "nstl/vector.h"
#include "nstl/hash.h"
#include "nstl/vector_hash.h"

#define DEFINE_HASH(T) template<> struct nstl::hash<T> { size_t operator()(T const& value) const { return value.hash(); } }
#define DEFINE_ENUM_HASH(T) template<> struct nstl::hash<T> { size_t operator()(T const& value) const { return nstl::hash<size_t>{}(static_cast<size_t>(value)); } }

// TODO split into several files?
namespace vkgfx
{
    enum class AttributeType
    {
        Vec2f,
        Vec3f,
        Vec4f,
        UInt32,
        Mat2f,
        Mat3f,
        Mat4f,
    };

    enum class VertexTopology
    {
        Triangles,
        TriangleStrip,
        TriangleFan,
    };
}

DEFINE_ENUM_HASH(vkgfx::AttributeType);
DEFINE_ENUM_HASH(vkgfx::VertexTopology);

namespace vkgfx
{
    struct UniformConfiguration
    {
        bool hasBuffer = true;
        bool hasAlbedoTexture = true;
        bool hasNormalMap = false;
        bool hasShadowMap = false;

        auto operator<=>(UniformConfiguration const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::UniformConfiguration);

namespace vkgfx
{
    struct VertexConfiguration
    {
        struct Binding
        {
            size_t stride = 0;

            auto operator<=>(Binding const&) const = default;

            size_t hash() const;
        };

        struct Attribute
        {
            size_t binding = 0;
            size_t location = 0;
            size_t offset = 0;
            AttributeType type = AttributeType::Vec4f;

            auto operator<=>(Attribute const&) const = default;

            size_t hash() const;
        };

        nstl::vector<Binding> bindings;
        nstl::vector<Attribute> attributes;
        VertexTopology topology;

        auto operator<=>(VertexConfiguration const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::VertexConfiguration::Binding);
DEFINE_HASH(vkgfx::VertexConfiguration::Attribute);
DEFINE_HASH(vkgfx::VertexConfiguration);

namespace vkgfx
{
    struct RenderConfiguration
    {
        // TODO merge with Pipeline::Config
        bool cullBackfaces = true;
        bool wireframe = false;
        bool depthTest = true;
        bool alphaBlending = false;
        bool depthBias = false;

        auto operator<=>(RenderConfiguration const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::RenderConfiguration);

namespace vkgfx
{
    struct PushConstantRange
    {
        size_t offset = 0;
        size_t size = 0;

        auto operator<=>(PushConstantRange const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::PushConstantRange);

namespace vkgfx
{
    struct DescriptorSetLayoutKey
    {
        UniformConfiguration uniformConfig;

        auto operator<=>(DescriptorSetLayoutKey const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::DescriptorSetLayoutKey);

namespace vkgfx
{
    struct PipelineLayoutKey
    {
        nstl::vector<UniformConfiguration> uniformConfigs;
        nstl::vector<PushConstantRange> pushConstantRanges;

        auto operator<=>(PipelineLayoutKey const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::PipelineLayoutKey);

namespace vkgfx
{
    struct PipelineKey
    {
        nstl::vector<ShaderModuleHandle> shaderHandles; // TODO rename to shaderModules
        nstl::vector<UniformConfiguration> uniformConfigs;
        VertexConfiguration vertexConfig;
        RenderConfiguration renderConfig;
        nstl::vector<PushConstantRange> pushConstantRanges;
        bool isShadowmap = false; // TODO remove this hack

        auto operator<=>(PipelineKey const&) const = default;

        size_t hash() const;
    };
}

DEFINE_HASH(vkgfx::PipelineKey);

#undef DEFINE_HASH
#undef DEFINE_ENUM_HASH

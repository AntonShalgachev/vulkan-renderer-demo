#pragma once

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

    struct UniformConfiguration
    {
        bool hasAlbedoTexture = true;
        bool hasNormalMap = false;

        auto operator<=>(UniformConfiguration const&) const = default;
    };

    struct VertexConfiguration
    {
        struct Binding
        {
            std::size_t stride = 0;

            auto operator<=>(Binding const&) const = default;
        };

        struct Attribute
        {
            std::size_t binding = 0;
            std::size_t location = 0;
            std::size_t offset = 0;
            AttributeType type = AttributeType::Vec4f;

            auto operator<=>(Attribute const&) const = default;
        };

        std::vector<Binding> bindings;
        std::vector<Attribute> attributes;
        VertexTopology topology;

        auto operator<=>(VertexConfiguration const&) const = default;
    };

    struct RenderConfiguration
    {
        // TODO merge with Pipeline::Config
        bool cullBackfaces = true;
        bool wireframe = false;
        bool depthTest = true;
        bool alphaBlending = false;
        bool dynamicScissor = false;

        auto operator<=>(RenderConfiguration const&) const = default;
    };

    struct PushConstantRange
    {
        std::size_t offset = 0;
        std::size_t size = 0;

        auto operator<=>(PushConstantRange const&) const = default;
    };

    struct PipelineKey
    {
        std::vector<ShaderModuleHandle> shaderHandles;
        std::vector<UniformConfiguration> uniformConfigs;
        VertexConfiguration vertexConfig;
        RenderConfiguration renderConfig;
        std::vector<PushConstantRange> pushConstantRanges;

        auto operator<=>(PipelineKey const&) const = default;
    };
}

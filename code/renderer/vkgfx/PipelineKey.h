#pragma once

namespace vkgfx
{
    enum class AttributeType
    {
        Vec2f,
        Vec3f,
        Vec4f,
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
    };

    struct VertexConfiguration
    {
        struct Binding
        {
            std::size_t stride = 0;
        };

        struct Attribute
        {
            std::size_t binding = 0;
            std::size_t location = 0;
            std::size_t offset = 0;
            AttributeType type = AttributeType::Vec4f;
        };

        std::vector<Binding> bindings;
        std::vector<Attribute> attributes;
        VertexTopology topology;
    };

    struct RenderConfiguration
    {
        bool cullBackfaces = true;
        bool wireframe = false;
    };

    struct PipelineKey
    {
        std::vector<ShaderModuleHandle> shaderHandles;
        UniformConfiguration uniformConfig;
        VertexConfiguration vertexConfig;
        RenderConfiguration renderConfig;
    };
}

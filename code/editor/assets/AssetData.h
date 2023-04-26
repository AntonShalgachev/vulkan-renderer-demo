#pragma once

// TODO move somewhere else

#include "editor/assets/Uuid.h"

#include "common/tiny_ctti.h"

#include "tglm/types.h"

#include "nstl/optional.h"
#include "nstl/vector.h"
#include "nstl/string.h"

namespace editor::assets
{
    // TODO move somewhere else?
    constexpr uint16_t materialAssetVersion = 1;
    constexpr uint16_t meshAssetVersion = 1;
    constexpr uint16_t sceneAssetVersion = 1;

    //////////////////////////////////////////////////////////////////////////
    // Material
    //////////////////////////////////////////////////////////////////////////

    enum class AlphaMode
    {
        Opaque,
        Mask,
        Blend,
    };
    TINY_CTTI_DESCRIBE_ENUM(AlphaMode, Opaque, Mask, Blend);

    enum class SamplerFilterMode
    {
        Nearest,
        Linear,
    };
    TINY_CTTI_DESCRIBE_ENUM(SamplerFilterMode, Nearest, Linear);

    enum class SamplerWrapMode
    {
        Repeat,
        Mirror,
        ClampToEdge,
    };
    TINY_CTTI_DESCRIBE_ENUM(SamplerWrapMode, Repeat, Mirror, ClampToEdge);

    struct SamplerData
    {
        SamplerFilterMode magFilter = SamplerFilterMode::Linear;
        SamplerFilterMode minFilter = SamplerFilterMode::Linear;
        SamplerWrapMode wrapU = SamplerWrapMode::Repeat;
        SamplerWrapMode wrapV = SamplerWrapMode::Repeat;
    };
    TINY_CTTI_DESCRIBE_STRUCT(SamplerData, magFilter, minFilter, wrapU, wrapV);

    struct TextureData
    {
        editor::assets::Uuid image;
        SamplerData sampler;
    };
    TINY_CTTI_DESCRIBE_STRUCT(TextureData, image, sampler);

    struct MaterialData
    {
        uint16_t version = 0;

        AlphaMode alphaMode = AlphaMode::Blend;
        float alphaCutoff = 0.0f;
        bool doubleSided = false;

        nstl::optional<tglm::vec4> baseColor;
        nstl::optional<TextureData> baseColorTexture;
        nstl::optional<TextureData> metallicRoughnessTexture;
        nstl::optional<TextureData> normalTexture;
    };
    TINY_CTTI_DESCRIBE_STRUCT(MaterialData, version, alphaMode, alphaCutoff, doubleSided, baseColor, baseColorTexture, metallicRoughnessTexture, normalTexture);

    //////////////////////////////////////////////////////////////////////////
    // Mesh
    //////////////////////////////////////////////////////////////////////////

    // TODO extend?
    enum class Topology
    {
        Lines,
        Triangles,
        TriangleStrip,
        TriangleFan,
    };
    TINY_CTTI_DESCRIBE_ENUM(Topology, Lines, Triangles, TriangleStrip, TriangleFan);

    enum class DataType
    {
        Scalar,
        Vec2,
        Vec3,
        Vec4,
        Mat2,
        Mat3,
        Mat4,
    };
    TINY_CTTI_DESCRIBE_ENUM(DataType, Scalar, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4);

    enum class DataComponentType
    {
        Int8,
        UInt8,
        Int16,
        UInt16,
        UInt32,
        Float,
    };
    TINY_CTTI_DESCRIBE_ENUM(DataComponentType, Int8, UInt8, Int16, UInt16, UInt32, Float);

    enum class AttributeSemantic
    {
        Position,
        Color,
        Normal,
        Tangent,
        Texcoord,
    };
    TINY_CTTI_DESCRIBE_ENUM(AttributeSemantic, Position, Color, Normal, Tangent, Texcoord);

    struct DataAccessorDescription
    {
        DataType type = DataType::Scalar;
        DataComponentType componentType = DataComponentType::Float;
        size_t count = 0;
        size_t stride = 0;
        size_t bufferOffset = 0;
    };
    TINY_CTTI_DESCRIBE_STRUCT(DataAccessorDescription, type, componentType, count, stride, bufferOffset);

    struct VertexAttributeDescription
    {
        AttributeSemantic semantic;
        size_t index = 0;
        DataAccessorDescription accessor;
    };
    TINY_CTTI_DESCRIBE_STRUCT(VertexAttributeDescription, semantic, index, accessor);

    struct PrimitiveDescription
    {
        editor::assets::Uuid material;
        Topology topology = Topology::Triangles;

        DataAccessorDescription indices;
        nstl::vector<VertexAttributeDescription> vertexAttributes;
    };
    TINY_CTTI_DESCRIBE_STRUCT(PrimitiveDescription, material, topology, indices, vertexAttributes);

    struct MeshData
    {
        uint16_t version = 0;
        nstl::vector<PrimitiveDescription> primitives;
    };
    TINY_CTTI_DESCRIBE_STRUCT(MeshData, version, primitives);

    //////////////////////////////////////////////////////////////////////////
    // Scene
    //////////////////////////////////////////////////////////////////////////

    // TODO generalize; these are basically "components"
    struct TransformParams
    {
        tglm::vec3 position = { 0, 0, 0 };
        tglm::vec3 scale = { 1, 1, 1 };
        tglm::quat rotation = tglm::quat::identity(); // TODO or euler angles?
    };
    TINY_CTTI_DESCRIBE_STRUCT(TransformParams, position, scale, rotation);

    struct MeshParams
    {
        editor::assets::Uuid id;
    };
    TINY_CTTI_DESCRIBE_STRUCT(MeshParams, id);

    enum class CameraType
    {
        Perspective,
        Orthographic,
    };
    TINY_CTTI_DESCRIBE_ENUM(CameraType, Perspective, Orthographic);

    struct PerspectiveCameraParams
    {
        float fov = 0.0f;
        nstl::optional<float> farZ;
        float nearZ = 0.0f;
    };
    TINY_CTTI_DESCRIBE_STRUCT(PerspectiveCameraParams, fov, farZ, nearZ);

    struct OrthographicCameraParams
    {
        float magX = 0.0f;
        float magY = 0.0f;
        float farZ = 0.0f;
        float nearZ = 0.0f;
    };
    TINY_CTTI_DESCRIBE_STRUCT(OrthographicCameraParams, magX, magY, farZ, nearZ);

    struct CameraParams
    {
        // TODO feels like std::variant
        CameraType type;
        nstl::optional<PerspectiveCameraParams> perspective;
        nstl::optional<OrthographicCameraParams> orthographic;
    };
    TINY_CTTI_DESCRIBE_STRUCT(CameraParams, type, perspective, orthographic);

    struct ObjectDescription
    {
        nstl::string name;

        nstl::optional<size_t> parentIndex;
        nstl::vector<size_t> childrenIndices;

        nstl::optional<TransformParams> transform;
        nstl::optional<MeshParams> mesh;
        nstl::optional<CameraParams> camera;
    };
    TINY_CTTI_DESCRIBE_STRUCT(ObjectDescription, name, parentIndex, childrenIndices, transform, mesh, camera);

    struct SceneData
    {
        uint16_t version = 0;
        nstl::vector<ObjectDescription> objects;
    };
    TINY_CTTI_DESCRIBE_STRUCT(SceneData, version, objects);
}

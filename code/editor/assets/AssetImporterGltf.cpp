#include "AssetImporterGltf.h"

#include "AssetDatabase.h"
#include "ImportDescription.h"

#include "memory/tracking.h"
#include "common/Utils.h"
#include "common/glm.h"
#include "common/tiny_ctti.h"
#include "common/json-tiny-ctti.h"
#include "common/json-nstl.h"
#include "logging/logging.h"
#include "path/path.h"

#include "fs/file.h"

#include "nstl/span.h"
#include "nstl/scope_exit.h"
#include "nstl/sprintf.h"

#include "yyjsoncpp/yyjsoncpp.h"

#include "cgltf.h"

namespace
{
    constexpr uint16_t materialAssetVersion = 1; // TODO move somewhere where the material would be also read
    constexpr uint16_t meshAssetVersion = 1; // TODO move somewhere where the mesh would be also read

    struct GltfResources
    {
        nstl::vector<nstl::blob> bufferData;

        nstl::vector<editor::assets::Uuid> images;
        nstl::vector<editor::assets::Uuid> materials;
        nstl::vector<editor::assets::Uuid> meshes;
    };

    template<typename T>
    size_t findIndex(T const* object, T const* first, size_t count)
    {
        auto index = object - first;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    }
}

namespace
{
    struct SamplerData
    {
        // TODO use enum
        int magFilter = 0;
        int minFilter = 0;
        int wrapU = 0;
        int wrapV = 0;
    };

    struct TextureData
    {
        editor::assets::Uuid image;
        SamplerData sampler;
    };

    enum class AlphaMode
    {
        Opaque,
        Mask,
        Blend,
    };

    struct MaterialData
    {
        uint16_t version = 0;

        AlphaMode alphaMode = AlphaMode::Blend;
        float alphaCutoff = 0.0f;
        bool doubleSided = false;

        nstl::optional<glm::vec4> baseColor;
        nstl::optional<TextureData> baseColorTexture;
        nstl::optional<TextureData> metallicRoughnessTexture;
        nstl::optional<TextureData> normalTexture;
    };
}

TINY_CTTI_DESCRIBE_STRUCT(SamplerData, magFilter, minFilter, wrapU, wrapV);
TINY_CTTI_DESCRIBE_STRUCT(TextureData, image, sampler);
TINY_CTTI_DESCRIBE_ENUM(AlphaMode, Opaque, Mask, Blend);
TINY_CTTI_DESCRIBE_STRUCT(MaterialData, version, alphaMode, alphaCutoff, doubleSided, baseColor, baseColorTexture, metallicRoughnessTexture, normalTexture);

namespace
{
    // TODO extend?
    enum class Topology
    {
        Lines,
        Triangles,
        TriangleStrip,
        TriangleFan,
    };

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

    enum class DataComponentType
    {
        Int8,
        UInt8,
        Int16,
        UInt16,
        UInt32,
        Float,
    };

    enum class AttributeSemantic
    {
        Position,
        Normal,
        Tangent,
        Texcoord,
    };

    struct DataAccessorDescription
    {
        DataType type = DataType::Scalar;
        DataComponentType componentType = DataComponentType::Float;
        size_t count = 0;
        size_t stride = 0;
        size_t bufferOffset = 0;
    };

    struct VertexAttributeDescription
    {
        AttributeSemantic semantic;
        size_t index = 0;
        DataAccessorDescription accessor;
    };

    struct PrimitiveDescription
    {
        editor::assets::Uuid material;
        Topology topology = Topology::Triangles;

        DataAccessorDescription indices;
        nstl::vector<VertexAttributeDescription> vertexAttributes;
    };

    struct MeshData
    {
        uint16_t version = 0;
        nstl::vector<PrimitiveDescription> primitives;
    };
}

TINY_CTTI_DESCRIBE_ENUM(Topology, Lines, Triangles, TriangleStrip, TriangleFan);
TINY_CTTI_DESCRIBE_ENUM(DataType, Scalar, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4);
TINY_CTTI_DESCRIBE_ENUM(DataComponentType, Int8, UInt8, Int16, UInt16, UInt32, Float);
TINY_CTTI_DESCRIBE_ENUM(AttributeSemantic, Position, Normal, Tangent, Texcoord);
TINY_CTTI_DESCRIBE_STRUCT(DataAccessorDescription, type, componentType, count, stride, bufferOffset);
TINY_CTTI_DESCRIBE_STRUCT(VertexAttributeDescription, semantic, index, accessor);
TINY_CTTI_DESCRIBE_STRUCT(PrimitiveDescription, material, topology, indices, vertexAttributes);
TINY_CTTI_DESCRIBE_STRUCT(MeshData, version, primitives);

namespace
{
    AlphaMode getAlphaMode(cgltf_alpha_mode mode)
    {
        switch (mode)
        {
        case cgltf_alpha_mode_opaque: return AlphaMode::Opaque;
        case cgltf_alpha_mode_mask: return AlphaMode::Mask;
        case cgltf_alpha_mode_blend: return AlphaMode::Blend;
        }

        assert(false);
        return AlphaMode::Opaque;
    }

    editor::assets::Uuid importImage(size_t i, cgltf_data const& data, editor::assets::ImportDescription const& desc, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        cgltf_image const& image = data.images[i];

        if (image.uri)
        {
            nstl::string_view mimeType = image.mime_type;

            nstl::string_view uri = image.uri;
            assert(!uri.starts_with("data:")); // TODO implement

            nstl::vector<editor::assets::Uuid> importedImages = database.importAsset(path::join(desc.parentDirectory, uri));
            assert(importedImages.size() == 1);
            return importedImages[0];
        }
        else if (image.buffer_view)
        {
            assert(false); // TODO implement
            return {};
        }

        assert(false);
        return {};
    }

    editor::assets::Uuid importMaterial(size_t i, cgltf_data const& data, editor::assets::ImportDescription const& desc, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        cgltf_material const& material = data.materials[i];

        auto createTextureData = [&resources, &data](cgltf_texture const& texture)
        {
            TextureData textureData;

            size_t imageIndex = findIndex(texture.image, data.images, data.images_count);
            textureData.image = resources.images[imageIndex];

            assert(texture.sampler);
            cgltf_sampler const& sampler = *texture.sampler;
            textureData.sampler.magFilter = sampler.mag_filter;
            textureData.sampler.minFilter = sampler.min_filter;
            textureData.sampler.wrapU = sampler.wrap_s;
            textureData.sampler.wrapV = sampler.wrap_t;

            return textureData;
        };

        MaterialData materialData;

        materialData.version = materialAssetVersion;

        materialData.alphaMode = getAlphaMode(material.alpha_mode);
        materialData.alphaCutoff = material.alpha_cutoff;
        materialData.doubleSided = material.double_sided;

        if (material.has_pbr_metallic_roughness)
        {
            materialData.baseColor = glm::make_vec4(material.pbr_metallic_roughness.base_color_factor);

            if (auto texture = material.pbr_metallic_roughness.base_color_texture.texture)
                materialData.baseColorTexture = createTextureData(*texture);
            if (auto texture = material.pbr_metallic_roughness.metallic_roughness_texture.texture)
                materialData.metallicRoughnessTexture = createTextureData(*texture);
        }

        if (auto texture = material.normal_texture.texture)
            materialData.normalTexture = createTextureData(*texture);

        nstl::string serializedMaterial;
        {
            namespace json = yyjsoncpp;

            json::mutable_doc doc;
            json::mutable_value_ref root = doc.create_value(materialData);
            doc.set_root(root);

            serializedMaterial = doc.write(json::write_flags::pretty);
        }

        nstl::string name = material.name ? material.name : nstl::sprintf("%.*s material %zu", desc.name.slength(), desc.name.data(), i);
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Material, name);
        database.addAssetFile(id, serializedMaterial, "material.json");

        return id;
    }

    DataComponentType getDataComponentType(cgltf_component_type componentType)
    {
        switch (componentType)
        {
        case cgltf_component_type_r_8: return DataComponentType::Int8;
        case cgltf_component_type_r_8u: return DataComponentType::UInt8;
        case cgltf_component_type_r_16: return DataComponentType::Int16;
        case cgltf_component_type_r_16u: return DataComponentType::UInt16;
        case cgltf_component_type_r_32u: return DataComponentType::UInt32;
        case cgltf_component_type_r_32f: return DataComponentType::Float;
        }

        assert(false);
        return DataComponentType::Float;
    }

    DataType getDataType(cgltf_type attributeType)
    {
        switch (attributeType)
        {
        case cgltf_type_scalar: return DataType::Scalar;
        case cgltf_type_vec2: return DataType::Vec2;
        case cgltf_type_vec3: return DataType::Vec3;
        case cgltf_type_vec4: return DataType::Vec4;
        case cgltf_type_mat2: return DataType::Mat2;
        case cgltf_type_mat3: return DataType::Mat3;
        case cgltf_type_mat4: return DataType::Mat4;
        }

        assert(false);
        return DataType::Scalar;
    }

    Topology getTopology(cgltf_primitive_type type)
    {
        switch (type)
        {
        case cgltf_primitive_type_lines: return Topology::Lines;
        case cgltf_primitive_type_triangles: return Topology::Triangles;
        case cgltf_primitive_type_triangle_strip: return Topology::TriangleStrip;
        case cgltf_primitive_type_triangle_fan: return Topology::TriangleFan;

        case cgltf_primitive_type_points:
        case cgltf_primitive_type_line_loop:
        case cgltf_primitive_type_line_strip:
            assert(false);
        }

        assert(false);
        return Topology::Triangles;
    }

    size_t getComponentSize(DataComponentType type)
    {
        switch (type)
        {
        case DataComponentType::Int8: return 1;
        case DataComponentType::UInt8: return 1;
        case DataComponentType::Int16: return 2;
        case DataComponentType::UInt16: return 2;
        case DataComponentType::UInt32: return 4;
        case DataComponentType::Float: return 4;
        }

        assert(false);
        return 0;
    }

    size_t getComponentsCount(DataType type)
    {
        switch (type)
        {
        case DataType::Scalar: return 1;
        case DataType::Vec2: return 2;
        case DataType::Vec3: return 3;
        case DataType::Vec4: return 4;
        case DataType::Mat2: return 4;
        case DataType::Mat3: return 9;
        case DataType::Mat4: return 16;
        }

        assert(false);
        return 0;
    }

    AttributeSemantic getAttributeSemantic(cgltf_attribute_type type)
    {
        switch (type)
        {
        case cgltf_attribute_type_position: return AttributeSemantic::Position;
        case cgltf_attribute_type_normal: return AttributeSemantic::Normal;
        case cgltf_attribute_type_tangent: return AttributeSemantic::Tangent;
        case cgltf_attribute_type_texcoord: return AttributeSemantic::Texcoord;

        case cgltf_attribute_type_color:
        case cgltf_attribute_type_joints:
        case cgltf_attribute_type_weights:
        case cgltf_attribute_type_custom:
            assert(false); // Not implemented yet
        }

        assert(false);
        return AttributeSemantic::Position;
    }

    void* memcpy_stride(void* destination, void const* source, size_t count, size_t chunk_size, size_t dst_stride, size_t src_stride)
    {
        if (dst_stride == chunk_size && src_stride == chunk_size)
            memcpy(destination, source, count * chunk_size);

        assert(dst_stride > 0);
        assert(src_stride > 0);

        char* dst = static_cast<char*>(destination);
        char const* src = static_cast<char const*>(source);

        for (size_t i = 0; i < count; i++)
        {
            memcpy(dst, src, chunk_size);

            dst += dst_stride;
            src += src_stride;
        }

        return destination;
    }

    struct DataLayout
    {
        size_t bufferIndex = 0;

        DataType type = DataType::Scalar;
        DataComponentType componentType = DataComponentType::Float;

        size_t count = 0;
        size_t elementSize = 0;
        size_t byteSize = 0;
        
        size_t offset = 0;
        size_t stride = 0;
    };

    DataLayout calculateDataLayout(cgltf_accessor const& accessor, cgltf_data const& data)
    {
        DataLayout layout{};

        cgltf_buffer_view const& bufferView = *accessor.buffer_view;
        layout.bufferIndex = findIndex(bufferView.buffer, data.buffers, data.buffers_count);

        layout.type = getDataType(accessor.type);
        layout.componentType = getDataComponentType(accessor.component_type);

        layout.count = accessor.count;
        layout.elementSize = getComponentSize(layout.componentType) * getComponentsCount(layout.type);
        layout.byteSize = layout.count * layout.elementSize;

        layout.offset = bufferView.offset + accessor.offset;
        layout.stride = bufferView.stride > 0 ? bufferView.stride : layout.elementSize;

        return layout;
    }

    struct PrimitiveParams
    {
        size_t totalByteSize = 0;
        size_t vertexElementByteSize = 0;

        DataLayout indexLayout;
        nstl::vector<DataLayout> attributeLayouts;
    };

    PrimitiveParams calculatePrimitiveParameters(cgltf_primitive const& primitive, cgltf_data const& data)
    {
        PrimitiveParams params{};

        params.indexLayout = calculateDataLayout(*primitive.indices, data);
        params.totalByteSize += params.indexLayout.byteSize;

        params.attributeLayouts.reserve(primitive.attributes_count);
        for (size_t i = 0; i < primitive.attributes_count; i++)
        {
            DataLayout layout = calculateDataLayout(*primitive.attributes[i].data, data);
            params.totalByteSize += layout.byteSize;
            params.vertexElementByteSize += layout.elementSize;

            params.attributeLayouts.push_back(layout);
        }

        return params;
    }

    PrimitiveDescription appendPrimitive(cgltf_primitive const& primitive, nstl::span<unsigned char> destinationData, cgltf_data const& data, GltfResources const& resources, PrimitiveParams const& params)
    {
        auto appendChunk = [&data, &resources, &destinationData](cgltf_accessor const& accessor, DataLayout const& layout, size_t offset, size_t stride)
        {
            nstl::blob const& sourceData = resources.bufferData[layout.bufferIndex];

            size_t count = layout.count;
            size_t elementSize = layout.elementSize;
            size_t sourceOffset = layout.offset;
            size_t sourceStride = layout.stride;

            size_t destinationOffset = offset;
            size_t destinationStride = stride > 0 ? stride : elementSize;

            assert(sourceOffset + sourceStride * (count - 1) + elementSize <= sourceData.size());
            assert(destinationOffset + destinationStride * (count - 1) + elementSize <= destinationData.size());
            memcpy_stride(destinationData.data() + destinationOffset, sourceData.cdata() + sourceOffset, count, elementSize, destinationStride, sourceStride);

            DataAccessorDescription result;

            result.type = layout.type;
            result.componentType = layout.componentType;
            result.count = layout.count;
            result.stride = destinationStride;
            result.bufferOffset = destinationOffset;

            return result;
        };

        PrimitiveDescription description;

        size_t materialIndex = findIndex(primitive.material, data.materials, data.materials_count);

        description.material = resources.materials[materialIndex];
        description.topology = getTopology(primitive.type);

        size_t indexOffset = 0;
        size_t indexStride = 0;

        assert(params.indexLayout.type == DataType::Scalar);
        description.indices = appendChunk(*primitive.indices, params.indexLayout, indexOffset, indexStride);

        size_t verticesOffset = params.indexLayout.byteSize;
        size_t verticesStride = params.vertexElementByteSize;

        assert(params.attributeLayouts.size() == primitive.attributes_count);
        for (size_t i = 0; i < primitive.attributes_count; i++)
        {
            cgltf_attribute const& attribute = primitive.attributes[i];

            DataLayout const& layout = params.attributeLayouts[i];

            VertexAttributeDescription& vertexAttributeDescription = description.vertexAttributes.emplace_back();
            vertexAttributeDescription.semantic = getAttributeSemantic(attribute.type);
            vertexAttributeDescription.index = attribute.index;

            vertexAttributeDescription.accessor = appendChunk(*attribute.data, layout, verticesOffset, verticesStride);

            verticesOffset += layout.elementSize;
        }

        return description;
    }

    editor::assets::Uuid importMesh(size_t i, cgltf_data const& data, editor::assets::ImportDescription const& desc, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        cgltf_mesh const& mesh = data.meshes[i];

        nstl::vector<PrimitiveParams> primitiveParams;
        size_t meshByteSize = 0;

        primitiveParams.reserve(mesh.primitives_count);
        for (size_t i = 0; i < mesh.primitives_count; i++)
        {
            PrimitiveParams params = calculatePrimitiveParameters(mesh.primitives[i], data);
            meshByteSize += params.totalByteSize;
            primitiveParams.push_back(params);
        }

        nstl::blob destinationData{ meshByteSize };

        nstl::vector<PrimitiveDescription> primitives;
        primitives.reserve(mesh.primitives_count);
        assert(primitiveParams.size() == mesh.primitives_count);

        for (size_t i = 0; i < mesh.primitives_count; i++)
            primitives.push_back(appendPrimitive(mesh.primitives[i], destinationData, data, resources, primitiveParams[i]));

        MeshData meshData = {
            .version = meshAssetVersion,
            .primitives = nstl::move(primitives),
        };

        nstl::string serializedMesh;
        {
            namespace json = yyjsoncpp;

            json::mutable_doc doc;
            json::mutable_value_ref root = doc.create_value(meshData);
            doc.set_root(root);

            serializedMesh = doc.write(json::write_flags::pretty);
        }

        nstl::string name = mesh.name ? mesh.name : nstl::sprintf("%.*s mesh %zu", desc.name.slength(), desc.name.data(), i);
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Mesh, name);
        database.addAssetFile(id, serializedMesh, "mesh.json");
        database.addAssetFile(id, destinationData, "buffer.bin");

        return id;
    }
}

namespace yyjsoncpp
{
    template<>
    struct serializer<glm::vec4>
    {
        static mutable_value_ref to_json(mutable_doc& doc, glm::vec4 const& value)
        {
            return doc.create_array(value.x, value.y, value.z, value.w);
        }
    };

    template<>
    struct serializer<editor::assets::Uuid>
    {
        static mutable_value_ref to_json(mutable_doc& doc, editor::assets::Uuid const& value)
        {
            return doc.create_string(value.toString());
        }
    };
}

editor::assets::AssetImporterGltf::AssetImporterGltf(AssetDatabase& database) : m_database(database)
{

}

nstl::vector<editor::assets::Uuid> editor::assets::AssetImporterGltf::importAsset(ImportDescription const& desc) const
{
    static auto scopeId = memory::tracking::create_scope_id("AssetImporter/GLTF");
    MEMORY_TRACKING_SCOPE(scopeId);

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result result = cgltf_parse(&options, desc.content.data(), desc.content.size(), &data);
    nstl::scope_exit freeCgltf = [&data]() { cgltf_free(data); data = nullptr; };

    if (result != cgltf_result_success)
        return {};

    if (!data)
        return {};

    return parseGltfData(*data, desc);
}

nstl::vector<editor::assets::Uuid> editor::assets::AssetImporterGltf::parseGltfData(cgltf_data const& data, ImportDescription const& desc) const
{
    nstl::vector<Uuid> result;

    GltfResources resources;

    for (size_t i = 0; i < data.extensions_required_count; i++)
        logging::warn("GLTF requires extension '{}'", data.extensions_required[i]);

    for (size_t i = 0; i < data.buffers_count; i++)
    {
       cgltf_buffer const& buffer = data.buffers[i];

       nstl::string_view uri = buffer.uri;

       bool isDataUri = uri.starts_with("data:");
       bool hasSchema = uri.find("://") != nstl::string_view::npos;

       assert(!isDataUri); // TODO implement
       assert(!hasSchema); // TODO implement?

       nstl::string path = path::join(desc.parentDirectory, uri);

       fs::file f;
       f.open(path, fs::open_mode::read);
       assert(f.is_open());

       nstl::blob data{ f.size() };
       f.read(data.data(), data.size());
       f.close();

       resources.bufferData.push_back(nstl::move(data));
    }

    result.reserve(result.size() + data.images_count);
    for (size_t i = 0; i < data.images_count; i++)
    {
        Uuid asset = importImage(i, data, desc, resources, m_database);
        resources.images.push_back(asset);
        result.push_back(asset);
    }

    for (size_t i = 0; i < data.materials_count; i++)
    {
        Uuid asset = importMaterial(i, data, desc, resources, m_database);
        resources.materials.push_back(asset);
        result.push_back(asset);
    }

    for (auto i = 0; i < data.meshes_count; i++)
    {
        Uuid asset = importMesh(i, data, desc, resources, m_database);
        resources.meshes.push_back(asset);
        result.push_back(asset);
    }
// 
//     for (size_t i = 0; i < data.cameras_count; i++)
//     {
//         cgltf_camera const& gltfCamera = data.cameras[i];
// 
//         if (gltfCamera.type == cgltf_camera_type_perspective)
//         {
//             cgltf_camera_perspective const& gltfParams = gltfCamera.data.perspective;
// 
//             assert(gltfParams.has_zfar);
//         }
//         else
//         {
//             assert(false);
//         }
//     }
// 
//     {
//         assert(data.scene);
//         m_demoScene = createDemoScene(model, *data.scene);
// 
//         qsort(m_demoScene.objects.data(), m_demoScene.objects.size(), sizeof(vkgfx::TestObject), [](void const* p1, void const* p2) -> int
//         {
//             vkgfx::TestObject const& lhs = *static_cast<vkgfx::TestObject const*>(p1);
//         vkgfx::TestObject const& rhs = *static_cast<vkgfx::TestObject const*>(p2);
// 
//         auto cmp = [](auto const& lhs, auto const& rhs) -> int
//         {
//             return (lhs > rhs) - (lhs < rhs);
//         };
// 
//         if (lhs.pipeline != rhs.pipeline)
//             return cmp(lhs.pipeline, rhs.pipeline);
// 
//         return cmp(lhs.material, rhs.material);
//         });
// 
//         if (!m_demoScene.cameras.empty())
//         {
//             DemoCamera const& camera = m_demoScene.cameras[0];
//             m_cameraTransform = camera.transform;
//             m_cameraParameters = m_gltfResources->cameraParameters[camera.parametersIndex];
//         }
//     }

    return result;
}

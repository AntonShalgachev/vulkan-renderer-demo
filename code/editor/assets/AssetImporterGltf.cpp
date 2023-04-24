#include "AssetImporterGltf.h"

#include "AssetDatabase.h"
#include "ImportDescription.h"
#include "AssetData.h"

#include "memory/tracking.h"
#include "common/Utils.h"
#include "common/json-tiny-ctti.h"
#include "common/json-nstl.h"
#include "common/json-glm.h"
#include "logging/logging.h"
#include "path/path.h"

#include "tglm/affine.h"

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
    constexpr uint16_t sceneAssetVersion = 1; // TODO move somewhere where the mesh would be also read

    struct GltfResources
    {
        nstl::vector<nstl::blob> bufferData;

        nstl::vector<editor::assets::Uuid> images;
        nstl::vector<editor::assets::Uuid> materials;
        nstl::vector<editor::assets::Uuid> meshes;
        nstl::vector<editor::assets::Uuid> scenes;
    };

    template<typename T>
    size_t findIndex(T const* object, T const* first, size_t count)
    {
        auto index = object - first;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    }

    template<typename T>
    nstl::string serializeToJson(T const& object)
    {
        namespace json = yyjsoncpp;

        json::mutable_doc doc;
        json::mutable_value_ref root = doc.create_value(object);
        doc.set_root(root);

        return doc.write(json::write_flags::pretty);
    }
}

// Textures
namespace
{
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
}

// Materials
namespace
{
    editor::assets::AlphaMode getAlphaMode(cgltf_alpha_mode mode)
    {
        switch (mode)
        {
        case cgltf_alpha_mode_opaque: return editor::assets::AlphaMode::Opaque;
        case cgltf_alpha_mode_mask: return editor::assets::AlphaMode::Mask;
        case cgltf_alpha_mode_blend: return editor::assets::AlphaMode::Blend;

        default:
            assert(false);
        }

        assert(false);
        return editor::assets::AlphaMode::Opaque;
    }

    editor::assets::SamplerFilterMode getFilterMode(cgltf_int mode)
    {
        // TODO remove magic numbers

        switch (mode)
        {
        case 9728: // NEAREST:
        case 9984: // NEAREST_MIPMAP_NEAREST:
        case 9986: // NEAREST_MIPMAP_LINEAR:
            return editor::assets::SamplerFilterMode::Nearest;

        case 0:
        case 9729: // LINEAR:
        case 9985: // LINEAR_MIPMAP_NEAREST:
        case 9987: // LINEAR_MIPMAP_LINEAR:
            return editor::assets::SamplerFilterMode::Linear;
        }

        assert(false);
        return editor::assets::SamplerFilterMode::Nearest;
    };

    editor::assets::SamplerWrapMode getWrapMode(cgltf_int mode)
    {
        // TODO remove magic numbers

        switch (mode)
        {
        case 10497: // REPEAT:
            return editor::assets::SamplerWrapMode::Repeat;
        case 33071: // CLAMP_TO_EDGE:
            return editor::assets::SamplerWrapMode::ClampToEdge;
        case 33648: // MIRRORED_REPEAT:
            return editor::assets::SamplerWrapMode::Mirror;
        }

        assert(false);
        return editor::assets::SamplerWrapMode::Repeat;
    };

    editor::assets::Uuid importMaterial(size_t i, cgltf_data const& data, editor::assets::ImportDescription const& desc, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        cgltf_material const& material = data.materials[i];

        auto createTextureData = [&resources, &data](cgltf_texture const& texture)
        {
            editor::assets::TextureData textureData;

            size_t imageIndex = findIndex(texture.image, data.images, data.images_count);
            textureData.image = resources.images[imageIndex];

            if (texture.sampler)
            {
                cgltf_sampler const& sampler = *texture.sampler;
                textureData.sampler.magFilter = getFilterMode(sampler.mag_filter);
                textureData.sampler.minFilter = getFilterMode(sampler.min_filter);
                textureData.sampler.wrapU = getWrapMode(sampler.wrap_s);
                textureData.sampler.wrapV = getWrapMode(sampler.wrap_t);
            }

            return textureData;
        };

        editor::assets::MaterialData materialData;

        materialData.version = materialAssetVersion;

        materialData.alphaMode = getAlphaMode(material.alpha_mode);
        materialData.alphaCutoff = material.alpha_cutoff;
        materialData.doubleSided = material.double_sided;

        if (material.has_pbr_metallic_roughness)
        {
            static_assert(sizeof(material.pbr_metallic_roughness.base_color_factor) == sizeof(tglm::vec4));
            materialData.baseColor = tglm::vec4(material.pbr_metallic_roughness.base_color_factor);

            if (auto texture = material.pbr_metallic_roughness.base_color_texture.texture)
                materialData.baseColorTexture = createTextureData(*texture);
            if (auto texture = material.pbr_metallic_roughness.metallic_roughness_texture.texture)
                materialData.metallicRoughnessTexture = createTextureData(*texture);
        }

        if (auto texture = material.normal_texture.texture)
            materialData.normalTexture = createTextureData(*texture);

        nstl::string name = material.name ? material.name : nstl::sprintf("%.*s material %zu", desc.name.slength(), desc.name.data(), i);
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Material, name);
        database.addAssetFile(id, serializeToJson(materialData), "material.json");

        return id;
    }
}

// Meshes
namespace
{
    editor::assets::DataComponentType getDataComponentType(cgltf_component_type componentType)
    {
        switch (componentType)
        {
        case cgltf_component_type_r_8: return editor::assets::DataComponentType::Int8;
        case cgltf_component_type_r_8u: return editor::assets::DataComponentType::UInt8;
        case cgltf_component_type_r_16: return editor::assets::DataComponentType::Int16;
        case cgltf_component_type_r_16u: return editor::assets::DataComponentType::UInt16;
        case cgltf_component_type_r_32u: return editor::assets::DataComponentType::UInt32;
        case cgltf_component_type_r_32f: return editor::assets::DataComponentType::Float;
        default:
            assert(false);
        }

        assert(false);
        return editor::assets::DataComponentType::Float;
    }

    editor::assets::DataType getDataType(cgltf_type attributeType)
    {
        switch (attributeType)
        {
        case cgltf_type_scalar: return editor::assets::DataType::Scalar;
        case cgltf_type_vec2: return editor::assets::DataType::Vec2;
        case cgltf_type_vec3: return editor::assets::DataType::Vec3;
        case cgltf_type_vec4: return editor::assets::DataType::Vec4;
        case cgltf_type_mat2: return editor::assets::DataType::Mat2;
        case cgltf_type_mat3: return editor::assets::DataType::Mat3;
        case cgltf_type_mat4: return editor::assets::DataType::Mat4;
        default:
            assert(false);
        }

        assert(false);
        return editor::assets::DataType::Scalar;
    }

    editor::assets::Topology getTopology(cgltf_primitive_type type)
    {
        switch (type)
        {
        case cgltf_primitive_type_lines: return editor::assets::Topology::Lines;
        case cgltf_primitive_type_triangles: return editor::assets::Topology::Triangles;
        case cgltf_primitive_type_triangle_strip: return editor::assets::Topology::TriangleStrip;
        case cgltf_primitive_type_triangle_fan: return editor::assets::Topology::TriangleFan;

        case cgltf_primitive_type_points:
        case cgltf_primitive_type_line_loop:
        case cgltf_primitive_type_line_strip:
            assert(false);
        default:
            assert(false);
        }

        assert(false);
        return editor::assets::Topology::Triangles;
    }

    size_t getComponentSize(editor::assets::DataComponentType type)
    {
        switch (type)
        {
        case editor::assets::DataComponentType::Int8: return 1;
        case editor::assets::DataComponentType::UInt8: return 1;
        case editor::assets::DataComponentType::Int16: return 2;
        case editor::assets::DataComponentType::UInt16: return 2;
        case editor::assets::DataComponentType::UInt32: return 4;
        case editor::assets::DataComponentType::Float: return 4;
        }

        assert(false);
        return 0;
    }

    size_t getComponentsCount(editor::assets::DataType type)
    {
        switch (type)
        {
        case editor::assets::DataType::Scalar: return 1;
        case editor::assets::DataType::Vec2: return 2;
        case editor::assets::DataType::Vec3: return 3;
        case editor::assets::DataType::Vec4: return 4;
        case editor::assets::DataType::Mat2: return 4;
        case editor::assets::DataType::Mat3: return 9;
        case editor::assets::DataType::Mat4: return 16;
        }

        assert(false);
        return 0;
    }

    editor::assets::AttributeSemantic getAttributeSemantic(cgltf_attribute_type type)
    {
        switch (type)
        {
        case cgltf_attribute_type_position: return editor::assets::AttributeSemantic::Position;
        case cgltf_attribute_type_normal: return editor::assets::AttributeSemantic::Normal;
        case cgltf_attribute_type_tangent: return editor::assets::AttributeSemantic::Tangent;
        case cgltf_attribute_type_texcoord: return editor::assets::AttributeSemantic::Texcoord;

        case cgltf_attribute_type_color:
        case cgltf_attribute_type_joints:
        case cgltf_attribute_type_weights:
        case cgltf_attribute_type_custom:
            assert(false); // Not implemented yet

        default:
            assert(false);
        }

        assert(false);
        return editor::assets::AttributeSemantic::Position;
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

        editor::assets::DataType type = editor::assets::DataType::Scalar;
        editor::assets::DataComponentType componentType = editor::assets::DataComponentType::Float;

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

    editor::assets::PrimitiveDescription appendPrimitive(cgltf_primitive const& primitive, nstl::span<unsigned char> destinationData, cgltf_data const& data, GltfResources const& resources, PrimitiveParams const& params)
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

            editor::assets::DataAccessorDescription result;

            result.type = layout.type;
            result.componentType = layout.componentType;
            result.count = layout.count;
            result.stride = destinationStride;
            result.bufferOffset = destinationOffset;

            return result;
        };

        editor::assets::PrimitiveDescription description;

        size_t materialIndex = findIndex(primitive.material, data.materials, data.materials_count);

        description.material = resources.materials[materialIndex];
        description.topology = getTopology(primitive.type);

        size_t indexOffset = 0;
        size_t indexStride = 0;

        assert(params.indexLayout.type == editor::assets::DataType::Scalar);
        description.indices = appendChunk(*primitive.indices, params.indexLayout, indexOffset, indexStride);

        size_t verticesOffset = params.indexLayout.byteSize;
        size_t verticesStride = params.vertexElementByteSize;

        assert(params.attributeLayouts.size() == primitive.attributes_count);
        for (size_t i = 0; i < primitive.attributes_count; i++)
        {
            cgltf_attribute const& attribute = primitive.attributes[i];

            DataLayout const& layout = params.attributeLayouts[i];

            editor::assets::VertexAttributeDescription& vertexAttributeDescription = description.vertexAttributes.emplace_back();
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

        nstl::vector<editor::assets::PrimitiveDescription> primitives;
        primitives.reserve(mesh.primitives_count);
        assert(primitiveParams.size() == mesh.primitives_count);

        for (size_t i = 0; i < mesh.primitives_count; i++)
            primitives.push_back(appendPrimitive(mesh.primitives[i], destinationData, data, resources, primitiveParams[i]));

        editor::assets::MeshData meshData = {
            .version = meshAssetVersion,
            .primitives = nstl::move(primitives),
        };

        nstl::string name = mesh.name ? mesh.name : nstl::sprintf("%.*s mesh %zu", desc.name.slength(), desc.name.data(), i);
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Mesh, name);
        database.addAssetFile(id, serializeToJson(meshData), "mesh.json");
        database.addAssetFile(id, destinationData, "buffer.bin");

        return id;
    }
}

// Scenes
namespace
{
    editor::assets::TransformParams createTransform(cgltf_node const& node)
    {
        if (node.has_matrix)
        {
            tglm::vec4 position;
            tglm::vec3 scale;
            tglm::quat rotation;
            tglm::decompose(tglm::mat4(node.matrix), position, rotation, scale);

            return editor::assets::TransformParams{ position, scale, rotation };
        }

        editor::assets::TransformParams transform;

        if (node.has_translation)
            transform.position = tglm::vec3(node.translation);

        if (node.has_scale)
            transform.scale = tglm::vec3(node.scale);

        if (node.has_rotation)
            transform.rotation = tglm::quat(node.rotation);

        return transform;
    }

    size_t addObjectsRecursive(cgltf_node const& node, cgltf_data const& data, editor::assets::SceneData& sceneData, GltfResources const& resources)
    {
        size_t objectDataIndex = sceneData.objects.size();

        // A reference to objectData might become invalid if sceneData.objects would relocate its items
        {
            editor::assets::ObjectDescription& objectData = sceneData.objects.emplace_back();

            if (node.name)
                objectData.name = node.name;
            else if (node.mesh && node.mesh->name)
                objectData.name = node.mesh->name;
            else if (node.camera && node.camera->name)
                objectData.name = node.camera->name;
            else if (node.light && node.light->name)
                objectData.name = node.light->name;
            else if (node.skin && node.skin->name)
                objectData.name = node.skin->name;
            else
                objectData.name = "Unnamed object";

            objectData.transform = createTransform(node);

            if (node.mesh)
            {
                size_t meshIndex = findIndex(node.mesh, data.meshes, data.meshes_count);
                objectData.mesh = editor::assets::MeshParams{ .id = resources.meshes[meshIndex], };
            }

            if (node.camera)
            {
                cgltf_camera const& camera = *node.camera;

                editor::assets::CameraParams cameraParams;

                if (camera.type == cgltf_camera_type_orthographic)
                {
                    cgltf_camera_orthographic const& params = camera.data.orthographic;

                    cameraParams.type = editor::assets::CameraType::Orthographic;
                    cameraParams.orthographic = editor::assets::OrthographicCameraParams{
                        .magX = params.xmag,
                        .magY = params.ymag,
                        .farZ = params.zfar,
                        .nearZ = params.znear,
                    };
                }
                else if (camera.type == cgltf_camera_type_perspective)
                {
                    cgltf_camera_perspective const& params = camera.data.perspective;

                    cameraParams.type = editor::assets::CameraType::Perspective;
                    cameraParams.perspective = editor::assets::PerspectiveCameraParams{
                        .fov = params.yfov,
                        .farZ = params.has_zfar ? params.zfar : nstl::optional<float>{},
                        .nearZ = params.znear,
                    };
                }
                else
                {
                    assert(false);
                }

                objectData.camera = cameraParams;
            }
        }

        sceneData.objects.reserve(sceneData.objects.size() + node.children_count);
        for (size_t i = 0; i < node.children_count; i++)
        {
            size_t childDataIndex = addObjectsRecursive(*node.children[i], data, sceneData, resources);

            editor::assets::ObjectDescription& childData = sceneData.objects[childDataIndex];
            editor::assets::ObjectDescription& objectData = sceneData.objects[objectDataIndex];

            assert(!childData.parentIndex);
            childData.parentIndex = childDataIndex;
            objectData.childrenIndices.push_back(childDataIndex);
        }

        return objectDataIndex;
    }

    editor::assets::Uuid importScene(size_t i, cgltf_data const& data, editor::assets::ImportDescription const& desc, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        assert(i < data.scenes_count);
        cgltf_scene const& scene = data.scenes[i];

        editor::assets::SceneData sceneData = {
            .version = sceneAssetVersion,
            .objects = {},
        };

        sceneData.objects.reserve(sceneData.objects.size() + scene.nodes_count);
        for (size_t index = 0; index < scene.nodes_count; index++)
            addObjectsRecursive(*scene.nodes[index], data, sceneData, resources);

        nstl::string name = scene.name ? scene.name : nstl::sprintf("%.*s scene %zu", desc.name.slength(), desc.name.data(), i);
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Scene, name);
        database.addAssetFile(id, serializeToJson(sceneData), "scene.json");

        return id;
    }
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

    for (auto i = 0; i < data.scenes_count; i++)
    {
        Uuid asset = importScene(i, data, desc, resources, m_database);
        resources.scenes.push_back(asset);
        result.push_back(asset);
    }

    return result;
}

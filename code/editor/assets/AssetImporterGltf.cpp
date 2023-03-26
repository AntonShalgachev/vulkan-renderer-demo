#include "AssetImporterGltf.h"

#include "AssetDatabase.h"
#include "ImportDescription.h"

#include "memory/tracking.h"
#include "common/Utils.h"
#include "common/glm.h"
#include "common/tiny_ctti.h"
#include "logging/logging.h"
#include "path/path.h"

#include "nstl/span.h"
#include "nstl/scope_exit.h"

#include "yyjsoncpp/yyjsoncpp.h"

#include "cgltf.h"

namespace
{
    struct GltfResources
    {
        nstl::vector<editor::assets::Uuid> images;
        nstl::vector<editor::assets::Uuid> materials;
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
        int magFilter;
        int minFilter;
        int wrapU;
        int wrapV;
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
        AlphaMode alphaMode;
        float alphaCutoff;
        bool doubleSided;

        nstl::optional<glm::vec4> baseColor;
        nstl::optional<TextureData> baseColorTexture;
        nstl::optional<TextureData> metallicRoughnessTexture;
        nstl::optional<TextureData> normalTexture;
    };
}

TINY_CTTI_DESCRIBE_STRUCT(SamplerData, magFilter, minFilter, wrapU, wrapV);
TINY_CTTI_DESCRIBE_STRUCT(TextureData, image, sampler);
TINY_CTTI_DESCRIBE_ENUM(AlphaMode, Opaque, Mask, Blend);
TINY_CTTI_DESCRIBE_STRUCT(MaterialData, alphaMode, alphaCutoff, doubleSided, baseColor, baseColorTexture, metallicRoughnessTexture, normalTexture);

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

    editor::assets::Uuid importImage(cgltf_image const& image, cgltf_data const& data, nstl::string_view parentDirectory, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
        //return {}; // TODO remove

        if (image.uri)
        {
            nstl::string_view mimeType = image.mime_type;

            nstl::string_view uri = image.uri;
            assert(!uri.starts_with("data:")); // TODO implement

            nstl::vector<editor::assets::Uuid> importedImages = database.importAsset(path::join(parentDirectory, uri));
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

    editor::assets::Uuid importMaterial(cgltf_material const& material, cgltf_data const& data, nstl::string_view parentDirectory, GltfResources const& resources, editor::assets::AssetDatabase& database)
    {
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

        nstl::string_view name = material.name ? material.name : "Gltf Material";
        editor::assets::Uuid id = database.createAsset(editor::assets::AssetType::Material, name);
        database.addAssetFile(id, serializedMaterial, "material.json");

        return id;
    }
}

namespace yyjsoncpp
{
    template<typename E>
    struct serializer<E, nstl::enable_if_t<tiny_ctti::is_enum_v<E>>>
    {
        static mutable_value_ref to_json(mutable_doc& doc, E const& mode)
        {
            nstl::string_view name = tiny_ctti::enum_name(mode);
            assert(!name.empty());
            return doc.create_string(name);
        }
    };

    template<typename T>
    struct serializer<T, nstl::enable_if_t<tiny_ctti::is_struct_v<T>>>
    {
        static mutable_value_ref to_json(mutable_doc& doc, T const& data)
        {
            mutable_object_ref root = doc.create_object();

            auto fields_to_json = [&data, &root](const auto&... entries)
            {
                ((root[entries.name] = data.*(entries.field)), ...);
            };

            nstl::apply(fields_to_json, tiny_ctti::struct_entries<T>());

            return root;
        }
    };

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

    template<typename T>
    struct serializer<nstl::optional<T>>
    {
        static mutable_value_ref to_json(mutable_doc& doc, nstl::optional<T> const& value)
        {
            if (value)
                return serializer<T>::to_json(doc, *value);
            else
                return doc.create_null();
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

    return parseGltfData(*data, desc.parentDirectory);
}

nstl::vector<editor::assets::Uuid> editor::assets::AssetImporterGltf::parseGltfData(cgltf_data const& data, nstl::string_view parentDirectory) const
{
    nstl::vector<Uuid> result;

    GltfResources resources;

    for (size_t i = 0; i < data.extensions_required_count; i++)
        logging::warn("GLTF requires extension '{}'", data.extensions_required[i]);

    //for (size_t i = 0; i < data.buffers_count; i++)
    //{
    //    cgltf_buffer const& gltfBuffer = data.buffers[i];

    //     assert(gltfBuffer.data);
    //     assert(gltfBuffer.size > 0);
 
    //     nstl::span<unsigned char const> data{ static_cast<unsigned char const*>(gltfBuffer.data), gltfBuffer.size };
    //}

    //for (size_t i = 0; i < data.samplers_count; i++)
    //{
    //    cgltf_sampler const& gltfSampler = data.samplers[i];

    //    auto convertFilterMode = [](int gltfMode)
    //    {
    //        // TODO remove magic numbers

    //        switch (gltfMode)
    //        {
    //        case 9728: // NEAREST:
    //        case 9984: // NEAREST_MIPMAP_NEAREST:
    //        case 9986: // NEAREST_MIPMAP_LINEAR:
    //            return vko::SamplerFilterMode::Nearest;

    //        case 0:
    //        case 9729: // LINEAR:
    //        case 9985: // LINEAR_MIPMAP_NEAREST:
    //        case 9987: // LINEAR_MIPMAP_LINEAR:
    //            return vko::SamplerFilterMode::Linear;
    //        }

    //        assert(false);
    //        return vko::SamplerFilterMode::Nearest;
    //    };

    //    auto convertWrapMode = [](int gltfMode)
    //    {
    //        // TODO remove magic numbers

    //        switch (gltfMode)
    //        {
    //        case 10497: // REPEAT:
    //            return vko::SamplerWrapMode::Repeat;
    //        case 33071: // CLAMP_TO_EDGE:
    //            return vko::SamplerWrapMode::ClampToEdge;
    //        case 33648: // MIRRORED_REPEAT:
    //            return vko::SamplerWrapMode::Mirror;
    //        }

    //        assert(false);
    //        return vko::SamplerWrapMode::Repeat;
    //    };

    //    auto magFilter = convertFilterMode(gltfSampler.mag_filter);
    //    auto minFilter = convertFilterMode(gltfSampler.min_filter);
    //    auto wrapU = convertWrapMode(gltfSampler.wrap_s);
    //    auto wrapV = convertWrapMode(gltfSampler.wrap_t);
    //}

    result.reserve(result.size() + data.images_count);
    for (size_t i = 0; i < data.images_count; i++)
    {
        Uuid asset = importImage(data.images[i], data, parentDirectory, resources, m_database);
        resources.images.push_back(asset);
        result.push_back(asset);
    }

    //for (size_t i = 0; i < data.textures_count; i++)
    //{
    //    cgltf_texture const& gltfTexture = data.textures[i];

    //    size_t imageIndex = findIndex(gltfTexture.image, data.images, data.images_count);
    //}

    for (size_t i = 0; i < data.materials_count; i++)
    {
        Uuid asset = importMaterial(data.materials[i], data, parentDirectory, resources, m_database);
        resources.materials.push_back(asset);
        result.push_back(asset);
    }

//     for (auto meshIndex = 0; meshIndex < data.meshes_count; meshIndex++)
//     {
//         cgltf_mesh const& gltfMesh = data.meshes[meshIndex];
// 
//         nstl::vector<DemoMesh>& demoMeshes = m_gltfResources->meshes.emplace_back();
//         demoMeshes.reserve(gltfMesh.primitives_count);
//         for (auto primitiveIndex = 0; primitiveIndex < gltfMesh.primitives_count; primitiveIndex++)
//         {
//             cgltf_primitive const& gltfPrimitive = gltfMesh.primitives[primitiveIndex];
// 
//             DemoMesh& demoMesh = demoMeshes.emplace_back();
// 
//             vkgfx::Mesh mesh;
// 
//             {
//                 auto findIndexType = [](int gltfComponentType)
//                 {
//                     switch (gltfComponentType)
//                     {
//                     case cgltf_component_type_r_8u:
//                         return vkgfx::IndexType::UnsignedByte;
//                     case cgltf_component_type_r_16u:
//                         return vkgfx::IndexType::UnsignedShort;
//                     case cgltf_component_type_r_32u:
//                         return vkgfx::IndexType::UnsignedInt;
//                     }
// 
//                     assert(false);
//                     return vkgfx::IndexType::UnsignedShort;
//                 };
// 
//                 cgltf_accessor const* gltfAccessor = gltfPrimitive.indices;
//                 assert(gltfAccessor);
//                 cgltf_buffer_view const* gltfBufferView = gltfAccessor->buffer_view;
//                 assert(gltfBufferView);
// 
//                 mesh.indexBuffer.buffer = m_gltfResources->buffers[findIndex(gltfBufferView->buffer, data.buffers, data.buffers_count)];
//                 mesh.indexBuffer.offset = gltfBufferView->offset + gltfAccessor->offset;
//                 mesh.indexCount = gltfAccessor->count;
//                 mesh.indexType = findIndexType(gltfAccessor->component_type);
//             }
// 
//             mesh.vertexBuffers.reserve(gltfPrimitive.attributes_count);
//             demoMesh.metadata.vertexConfig.bindings.reserve(gltfPrimitive.attributes_count);
//             demoMesh.metadata.vertexConfig.attributes.reserve(gltfPrimitive.attributes_count);
// 
//             for (size_t attributeIndex = 0; attributeIndex < gltfPrimitive.attributes_count; attributeIndex++)
//             {
//                 cgltf_attribute const& gltfAttribute = gltfPrimitive.attributes[attributeIndex];
// 
//                 nstl::string_view name = gltfAttribute.name;
// 
//                 nstl::optional<std::size_t> location = findAttributeLocation(name);
// 
//                 if (!location)
//                 {
//                     logging::warn("Skipping attribute '{}'", name);
//                     continue;
//                 }
// 
//                 cgltf_accessor const* gltfAccessor = gltfAttribute.data;
//                 assert(gltfAccessor);
//                 cgltf_buffer_view const* gltfBufferView = gltfAccessor->buffer_view;
//                 assert(gltfBufferView);
// 
//                 size_t bufferIndex = findIndex(gltfBufferView->buffer, data.buffers, data.buffers_count);
// 
//                 vkgfx::BufferWithOffset& attributeBuffer = mesh.vertexBuffers.emplace_back();
//                 attributeBuffer.buffer = m_gltfResources->buffers[bufferIndex];
//                 attributeBuffer.offset = gltfBufferView->offset + gltfAccessor->offset; // TODO can be improved
// 
//                 if (name == "COLOR_0")
//                     demoMesh.metadata.attributeSemanticsConfig.hasColor = true;
//                 if (name == "TEXCOORD_0")
//                     demoMesh.metadata.attributeSemanticsConfig.hasUv = true;
//                 if (name == "NORMAL")
//                     demoMesh.metadata.attributeSemanticsConfig.hasNormal = true;
//                 if (name == "TANGENT")
//                     demoMesh.metadata.attributeSemanticsConfig.hasTangent = true;
// 
//                 vkgfx::AttributeType attributeType = findAttributeType(gltfAccessor->type, gltfAccessor->component_type);
// 
//                 std::size_t stride = gltfBufferView->stride;
//                 if (stride == 0)
//                     stride = getAttributeByteSize(attributeType);
// 
//                 vkgfx::VertexConfiguration::Binding& bindingConfig = demoMesh.metadata.vertexConfig.bindings.emplace_back();
//                 bindingConfig.stride = stride;
// 
//                 vkgfx::VertexConfiguration::Attribute& attributeConfig = demoMesh.metadata.vertexConfig.attributes.emplace_back();
//                 attributeConfig.binding = attributeIndex;
//                 attributeConfig.location = *location;
//                 attributeConfig.offset = 0; // TODO can be improved
//                 attributeConfig.type = attributeType;
// 
//                 // TODO implement
//                 assert(gltfPrimitive.type == cgltf_primitive_type_triangles);
//                 demoMesh.metadata.vertexConfig.topology = vkgfx::VertexTopology::Triangles;
// 
//                 demoMesh.metadata.materialIndex = findIndex(gltfPrimitive.material, data.materials, data.materials_count); // TODO check
//             }
//         }
//     }
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

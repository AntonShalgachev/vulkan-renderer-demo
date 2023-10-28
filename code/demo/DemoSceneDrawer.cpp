#include "DemoSceneDrawer.h"

#include "gfx/resources.h"

#include "tiny_ktx/tiny_ktx.h"

#include "memory/tracking.h"

#include "fs/file.h"

#include "nstl/array.h"
#include "nstl/blob.h"

#include "dds-ktx.h"

#include <limits.h>
#include <vulkan/vulkan.h> // TODO remove

namespace
{
    struct ImageData
    {
        struct MipData
        {
            size_t offset = 0;
            size_t size = 0;
        };

        size_t width = 0;
        size_t height = 0;

        gfx::image_format format = gfx::image_format::r8g8b8a8;

        nstl::vector<MipData> mips;
    };

    nstl::optional<ImageData> loadWithDdspp(nstl::blob_view bytes)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load/DDS");
        MEMORY_TRACKING_SCOPE(scopeId);

        assert(bytes.size() <= INT_MAX);

        ddsktx_texture_info info{};
        if (!ddsktx_parse(&info, bytes.data(), static_cast<int>(bytes.size()), nullptr))
            return {};

        assert(info.width > 0);
        assert(info.height > 0);
        assert(info.bpp > 0);
        assert(info.bpp % 4 == 0);

        ImageData imageData;

        imageData.width = static_cast<size_t>(info.width);
        imageData.height = static_cast<size_t>(info.height);

        imageData.format = [](ddsktx_format format)
        {
            switch (format)
            {
            case DDSKTX_FORMAT_BC1:
                return gfx::image_format::bc1_unorm;
            case DDSKTX_FORMAT_BC3:
                return gfx::image_format::bc3_unorm;
            case DDSKTX_FORMAT_BC5:
                return gfx::image_format::bc5_unorm;
            default: // TODO implement other formats
                assert(false);
            }

            assert(false);
            return gfx::image_format::bc1_unorm;
        }(info.format);

        for (int mip = 0; mip < info.num_mips; mip++)
        {
            ddsktx_sub_data mipInfo;
            ddsktx_get_sub(&info, &mipInfo, bytes.data(), static_cast<int>(bytes.size()), 0, 0, mip);

            assert(mipInfo.buff > bytes.data());
            ptrdiff_t offset = static_cast<unsigned char const*>(mipInfo.buff) - bytes.ucdata();
            assert(offset >= 0);

            imageData.mips.push_back({ static_cast<size_t>(offset), static_cast<size_t>(mipInfo.size_bytes) });
        }

        return imageData;
    }

    nstl::optional<ImageData> loadWithKtx(fs::file& f)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load/KTX");
        MEMORY_TRACKING_SCOPE(scopeId);

        class memory_stream : public tiny_ktx::input_stream
        {
        public:
            memory_stream(fs::file& f) : m_file(f) {}

            bool read(void* dest, size_t size) override
            {
                if (!dest)
                    return false;

                if (!m_file.try_read(dest, size, m_position))
                    return false;

                m_position += size;
                return true;
            }

        private:
            fs::file& m_file;
            size_t m_position = 0;
        };

        memory_stream stream{ f };

        tiny_ktx::image_header header;
        if (!tiny_ktx::parse_header(&header, stream))
            return {};

        assert(header.layer_count == 0 || header.layer_count == 1);
        assert(header.face_count == 1);
        assert(header.pixel_depth == 0 || header.pixel_depth == 1);

        ImageData imageData;

        imageData.width = header.pixel_width;
        imageData.height = header.pixel_height;

        imageData.format = [](VkFormat format)
        {
            switch (format)
            {
            case VK_FORMAT_R8G8B8_UNORM:
                return gfx::image_format::r8g8b8;
            case VK_FORMAT_R8G8B8A8_UNORM:
                return gfx::image_format::r8g8b8a8;
            default: // TODO implement other formats
                assert(false);
            }

            assert(false);
            return gfx::image_format::r8g8b8a8;
        }(static_cast<VkFormat>(header.vk_format));

        size_t mipsCount = tiny_ktx::get_level_count(header);

        nstl::vector<tiny_ktx::image_level_info> levelIndex{ mipsCount }; // TODO: static or hybrid vector
        if (!tiny_ktx::load_image_level_index(levelIndex.data(), levelIndex.size(), header, stream))
            return {};

        size_t dataOffset = levelIndex[mipsCount - 1].byte_offset;
        [[maybe_unused]] size_t dataSize = levelIndex[0].byte_offset + levelIndex[0].byte_length - dataOffset;

//         assert(dataOffset + dataSize <= bytes.size());

        for (size_t i = 0; i < mipsCount; i++)
        {
            size_t mipOffset = levelIndex[i].byte_offset - dataOffset;
            size_t mipSize = levelIndex[i].byte_length;

            imageData.mips.push_back({ dataOffset + mipOffset, mipSize });
        }

        return imageData;
    }

    nstl::optional<ImageData> loadImage(nstl::string_view path)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load");
        MEMORY_TRACKING_SCOPE(scopeId);

        fs::file f{ path, fs::open_mode::read };

//         if (auto data = loadWithDdspp(f))
//             return data;

        if (auto data = loadWithKtx(f))
            return data;

        return {};
    }
}

DemoSceneDrawer::DemoSceneDrawer(gfx::renderer& renderer, gfx::renderpass_handle shadowRenderpass)
    : m_renderer(renderer)
    , m_shadowRenderpass(shadowRenderpass)
{
    static auto shadersScopeId = memory::tracking::create_scope_id("Scene/Load/Shader");

    m_defaultVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shader.vert");
    m_defaultFragmentShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shader.frag");
    m_shadowmapVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shadowmap.vert");

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    for (auto const& pair : m_defaultVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        m_shaders.insert_or_assign(modulePath, m_renderer.create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    for (auto const& pair : m_defaultFragmentShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        m_shaders.insert_or_assign(modulePath, m_renderer.create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::fragment,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_shadowmapVertexShader->getAll())
    for (auto const& pair : m_shadowmapVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        m_shaders.insert_or_assign(modulePath, m_renderer.create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    m_defaultSampler = m_renderer.create_sampler({});
}

DemoTexture* DemoSceneDrawer::createTexture(nstl::string_view path)
{
    // TODO leads to unnecessary data copy; change that
    nstl::optional<ImageData> imageData = loadImage(path);
    assert(imageData);

    assert(!imageData->mips.empty());
    assert(imageData->mips[0].size > 0);

    // TODO support all mips

    ImageData::MipData const& mipData = imageData->mips[0];

    struct file_reader : gfx::data_reader
    {
        file_reader(nstl::string_view path, size_t offset, size_t size) : m_offset(offset), m_size(size)
        {
            m_file.open(path, fs::open_mode::read);
            assert(m_offset + m_size <= m_file.size());
        }

        size_t get_size() const override { return m_size; }

        bool read(void* destination, size_t size) override
        {
            assert(size <= m_size);
            return m_file.try_read(destination, size, m_offset);
        }

        fs::file m_file;
        size_t m_offset = 0;
        size_t m_size = 0;
    };

    file_reader reader{ path, mipData.offset, mipData.size };

    auto image = m_renderer.create_image({
        .width = imageData->width,
        .height = imageData->height,
        .format = imageData->format,
        .type = gfx::image_type::color,
        .usage = gfx::image_usage::upload_sampled,
    });
    m_renderer.image_upload_sync(image, reader);

    m_textures.push_back(nstl::make_unique<DemoTexture>());
    DemoTexture* texture = m_textures.back().get();
    texture->image = image;

    return texture;
}

DemoMaterial* DemoSceneDrawer::createMaterial(tglm::vec4 color, DemoTexture* albedoTexture, DemoTexture* normalTexture, bool doubleSided)
{
    m_materials.push_back(nstl::make_unique<DemoMaterial>());
    DemoMaterial* material = m_materials.back().get();

    material->buffer = m_renderer.create_buffer({
        .size = sizeof(color),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = false,
    });

    m_renderer.buffer_upload_sync(material->buffer, { &color, sizeof(color) });

    nstl::vector<gfx::descriptorgroup_entry> descriptor_entries;
    nstl::vector<gfx::descriptor_layout_entry> descriptor_layout_entries;
    descriptor_entries.push_back({ 0, {material->buffer, gfx::descriptor_type::uniform_buffer} });
    descriptor_layout_entries.push_back({ 0, gfx::descriptor_type::uniform_buffer });

    if (albedoTexture)
    {
        // TODO create actual sampler
        descriptor_entries.push_back({ 1, {albedoTexture->image, m_defaultSampler} });
        descriptor_layout_entries.push_back({ 1, gfx::descriptor_type::combined_image_sampler });
    }

    if (normalTexture)
    {
        // TODO create actual sampler
        descriptor_entries.push_back({ 2, {normalTexture->image, m_defaultSampler} });
        descriptor_layout_entries.push_back({ 2, gfx::descriptor_type::combined_image_sampler });
    }

    material->descriptorGroupLayout = { descriptor_layout_entries };
    material->descriptorGroup = m_renderer.create_descriptorgroup({
        .entries = descriptor_entries,
    });

    material->hasAlbedoTexture = albedoTexture != nullptr;
    material->hasNormalTexture = normalTexture != nullptr;
    material->wireframe = false;
    material->cullBackfaces = !doubleSided;

    return material;
}

DemoMesh* DemoSceneDrawer::createMesh(nstl::blob_view bytes, nstl::span<PrimitiveParams> primitiveParams)
{
    m_meshes.push_back(nstl::make_unique<DemoMesh>());
    DemoMesh* mesh = m_meshes.back().get();

    mesh->buffer = m_renderer.create_buffer({
        .size = bytes.size(),
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::device_local,
        .is_mutable = false,
    });
    m_renderer.buffer_upload_sync(mesh->buffer, bytes);

    for (PrimitiveParams const& params : primitiveParams)
    {
        DemoPrimitive& demoPrimitive = mesh->primitives.emplace_back();

        demoPrimitive.material = params.material;
        demoPrimitive.indexBuffer = { mesh->buffer, params.indexBufferOffset };
        demoPrimitive.indexType = params.indexType;
        demoPrimitive.indexCount = params.indexCount;

        demoPrimitive.hasColor = params.hasColor;
        demoPrimitive.hasUv = params.hasUv;
        demoPrimitive.hasNormal = params.hasNormal;
        demoPrimitive.hasTangent = params.hasTangent;

        for (AttributeParams const& attributeParams : params.attributes)
        {
            demoPrimitive.vertexBuffers.push_back({ mesh->buffer, attributeParams.bufferOffset });

            // TODO probably a single buffer can be used
            size_t buffer_index = demoPrimitive.vertexConfig.buffer_bindings.size();
            demoPrimitive.vertexConfig.buffer_bindings.push_back({
                .buffer_index = buffer_index,
                .stride = attributeParams.stride,
            });

            size_t binding_index = buffer_index;
            demoPrimitive.vertexConfig.attributes.push_back({
                .location = attributeParams.location,
                .buffer_binding_index = binding_index,
                .offset = 0,
                .type = attributeParams.type,
            });
        }
    }

    return mesh;
}

void DemoSceneDrawer::addMeshInstance(DemoMesh* mesh, tglm::mat4 matrix, tglm::vec4 color)
{
    for (size_t i = 0; i < mesh->primitives.size(); i++)
    {
        DemoPrimitive& primitive = mesh->primitives[i];

        m_objects.push_back(nstl::make_unique<DemoObject>());
        DemoObject* object = m_objects.back().get();

        DemoMaterial* material = primitive.material;

        // TODO reimplement
        ShaderConfiguration shaderConfiguration;
        shaderConfiguration.hasTexture = material->hasAlbedoTexture;
        shaderConfiguration.hasNormalMap = material->hasNormalTexture;
        shaderConfiguration.hasColor = primitive.hasColor;
        shaderConfiguration.hasTexCoord = primitive.hasUv;
        shaderConfiguration.hasNormal = primitive.hasNormal;
        shaderConfiguration.hasTangent = primitive.hasTangent;

        nstl::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
        nstl::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);
        assert(vertexShaderPath);
        assert(fragmentShaderPath);

        object->defaultRenderstate = m_renderer.create_renderstate({
            .shaders = nstl::array{ m_shaders[*vertexShaderPath], m_shaders[*fragmentShaderPath] },
            .renderpass = m_renderer.get_main_renderpass(),
            .vertex_config = primitive.vertexConfig,
            .descriptorgroup_layouts = nstl::array{
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::uniform_buffer},
                        gfx::descriptor_layout_entry{1, gfx::descriptor_type::uniform_buffer},
                        gfx::descriptor_layout_entry{2, gfx::descriptor_type::combined_image_sampler},
                    },
                },
                material->descriptorGroupLayout,
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::uniform_buffer},
                    },
                },
            },
            .flags = {
                .cull_backfaces = material->cullBackfaces,
                .wireframe = material->wireframe,
            },
        });

        nstl::string const* shadowmapVertexShaderPath = m_shadowmapVertexShader->get({});
        assert(shadowmapVertexShaderPath);

        object->shadowRenderstate = m_renderer.create_renderstate({
            .shaders = nstl::array{ m_shaders[*shadowmapVertexShaderPath] },
            .renderpass = m_shadowRenderpass,
            .vertex_config = primitive.vertexConfig,
            .descriptorgroup_layouts = nstl::array{
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::uniform_buffer},
                    },
                },
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::uniform_buffer},
                    },
                },
            },
            .flags = {
                .depth_bias = true,
            },
        });

        struct ObjectUniformBuffer
        {
            tglm::mat4 matrix;
            tglm::vec4 color;
        };

        auto objectUniformBuffer = m_renderer.create_buffer({
            .size = sizeof(ObjectUniformBuffer),
            .usage = gfx::buffer_usage::uniform,
            .location = gfx::buffer_location::host_visible,
            .is_mutable = false,
        });

        {
            ObjectUniformBuffer values = {
                .matrix = matrix,
                .color = color,
            };

            m_renderer.buffer_upload_sync(objectUniformBuffer, { &values, sizeof(values) });
        }

        object->descriptorGroup = m_renderer.create_descriptorgroup({
            .entries = nstl::array{ gfx::descriptorgroup_entry{ 0, {objectUniformBuffer, gfx::descriptor_type::uniform_buffer} } },
        });

        object->mesh = mesh;
        object->primitiveIndex = i;
    }
}

void DemoSceneDrawer::updateResources()
{

}

void DemoSceneDrawer::draw(bool shadow, gfx::descriptorgroup_handle defaultFrameDescriptorGroup, gfx::descriptorgroup_handle shadowFrameDescriptorGroup)
{
    for (auto const& objectPtr : m_objects)
    {
        DemoObject const& object = *objectPtr;
        DemoPrimitive& primitive = object.mesh->primitives[object.primitiveIndex];

        nstl::array defaultDescriptorGroups = { defaultFrameDescriptorGroup, primitive.material->descriptorGroup, object.descriptorGroup };
        nstl::array shadowDescriptorGroups = { shadowFrameDescriptorGroup, object.descriptorGroup };
        nstl::span<gfx::descriptorgroup_handle const> defaultDescriptorGroupsView = defaultDescriptorGroups;
        nstl::span<gfx::descriptorgroup_handle const> shadowDescriptorGroupsView = shadowDescriptorGroups;

        m_renderer.draw_indexed({
            .renderstate = shadow ? object.shadowRenderstate : object.defaultRenderstate,
            .descriptorgroups = shadow ? shadowDescriptorGroupsView : defaultDescriptorGroupsView,

            .vertex_buffers = primitive.vertexBuffers,
            .index_buffer = primitive.indexBuffer,
            .index_type = primitive.indexType,

            .index_count = primitive.indexCount,
            .first_index = 0,
            .vertex_offset = 0,
        });
    }
}

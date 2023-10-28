#pragma once

#include "ShaderPackage.h"

#include "gfx/renderer.h"

#include "nstl/blob_view.h"
#include "nstl/span.h"
#include "nstl/unique_ptr.h"
#include "nstl/unordered_map.h"
#include "nstl/vector.h"

struct DemoTexture
{
    gfx::image_handle image;
};

struct DemoMaterial
{
    gfx::buffer_handle buffer;
    gfx::descriptorgroup_handle descriptorGroup;

    gfx::descriptorgroup_layout_storage descriptorGroupLayout;
    gfx::renderstate_flags renderstateFlags;

    bool hasAlbedoTexture = false;
    bool hasNormalTexture = false;
    bool cullBackfaces = false;
    bool wireframe = false;
};

struct DemoPrimitive
{
    DemoMaterial* material = nullptr;

    nstl::vector<gfx::buffer_with_offset> vertexBuffers;
    gfx::buffer_with_offset indexBuffer;
    gfx::index_type indexType = gfx::index_type::uint16;
    size_t indexCount = 0;

    gfx::vertex_configuration_storage vertexConfig;

    bool hasColor = false;
    bool hasUv = false;
    bool hasNormal = false;
    bool hasTangent = false;
};

struct DemoMesh
{
    gfx::buffer_handle buffer;
    nstl::vector<DemoPrimitive> primitives;
};

struct DemoObject
{
    gfx::renderstate_handle defaultRenderstate;
    gfx::renderstate_handle shadowRenderstate;
    gfx::descriptorgroup_handle descriptorGroup;

    DemoMesh* mesh = nullptr;
    size_t primitiveIndex = 0;
};

class DemoSceneDrawer
{
public:
    DemoSceneDrawer(gfx::renderer& renderer, gfx::renderpass_handle shadowRenderpass);

    DemoTexture* createTexture(nstl::string_view path);
    DemoMaterial* createMaterial(tglm::vec4 color, DemoTexture* albedoTexture, DemoTexture* normalTexture, bool doubleSided);

    struct AttributeParams
    {
        size_t location = 0;
        size_t bufferOffset = 0;
        size_t stride = 0;
        gfx::attribute_type type = gfx::attribute_type::vec4f;
    };
    struct PrimitiveParams
    {
        gfx::vertex_topology topology = gfx::vertex_topology::triangles;
        DemoMaterial* material = nullptr;

        size_t indexBufferOffset = 0;
        gfx::index_type indexType = gfx::index_type::uint16;
        size_t indexCount = 0;

        nstl::vector<AttributeParams> attributes;

        bool hasColor = false;
        bool hasUv = false;
        bool hasNormal = false;
        bool hasTangent = false;
    };
    DemoMesh* createMesh(nstl::blob_view bytes, nstl::span<PrimitiveParams> params);

    void addMeshInstance(DemoMesh* mesh, tglm::mat4 matrix, tglm::vec4 color);

    void updateResources();
    void draw(bool shadow, gfx::descriptorgroup_handle defaultFrameDescriptorGroup, gfx::descriptorgroup_handle shadowFrameDescriptorGroup);

private:
    gfx::renderer& m_renderer;
    gfx::renderpass_handle m_shadowRenderpass;

    nstl::unique_ptr<ShaderPackage> m_defaultVertexShader;
    nstl::unique_ptr<ShaderPackage> m_defaultFragmentShader;
    nstl::unique_ptr<ShaderPackage> m_shadowmapVertexShader;
    nstl::unordered_map<nstl::string, gfx::shader_handle> m_shaders;

    gfx::sampler_handle m_defaultSampler;
//     gfx::buffer_handle m_viewProjectionData;
//     gfx::buffer_handle m_lightData;
//     gfx::buffer_handle m_shadowmapViewProjectionData;
//     gfx::descriptorgroup_handle m_shadowmapCameraDescriptorGroup;
//     gfx::renderpass_handle m_shadowRenderpass;
//     gfx::image_handle m_shadowImage;
//     gfx::framebuffer_handle m_shadowFramebuffer;
//     gfx::descriptorgroup_handle m_cameraDescriptorGroup;

    nstl::vector<nstl::unique_ptr<DemoTexture>> m_textures;
    nstl::vector<nstl::unique_ptr<DemoMaterial>> m_materials;
    nstl::vector<nstl::unique_ptr<DemoMesh>> m_meshes;
    nstl::vector<nstl::unique_ptr<DemoObject>> m_objects;
};

#include "DebugDrawService.h"

#include "vkgfx/ResourceManager.h"
#include "vkgfx/Renderer.h"
#include "vkgfx/BufferMetadata.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/PipelineKey.h"

#include "vkgfx/TestObject.h"

#include "ShaderPackage.h"

#include "vko/ShaderModuleProperties.h"

#include "common/Utils.h"

#include "gfx/renderer.h"

#include "tglm/types.h"
#include "tglm/affine.h"

#include "memory/tracking.h"

#include "nstl/array.h"
#include "nstl/vector.h"

namespace
{
    auto debugDrawScopeId = memory::tracking::create_scope_id("Rendering/DebugDraw");

    struct BoxVertex
    {
        tglm::vec3 position;
        tglm::vec3 normal;
    };

    BoxVertex boxVertices[] = {
        { {-0.5, -0.5, 0.5}, {0, 0, 1} },
        { {0.5, -0.5, 0.5}, {0, 0, 1} },
        { {-0.5, 0.5, 0.5}, {0, 0, 1} },
        { {0.5, 0.5, 0.5}, {0, 0, 1} },
        { {0.5, -0.5, 0.5}, {0, -1, 0} },
        { {-0.5, -0.5, 0.5}, {0, -1, 0} },
        { {0.5, -0.5, -0.5}, {0, -1, 0} },
        { {-0.5, -0.5, -0.5}, {0, -1, 0} },
        { {0.5, 0.5, 0.5}, {1, 0, 0} },
        { {0.5, -0.5, 0.5}, {1, 0, 0} },
        { {0.5, 0.5, -0.5}, {1, 0, 0} },
        { {0.5, -0.5, -0.5}, {1, 0, 0} },
        { {-0.5, 0.5, 0.5}, {0, 1, 0} },
        { {0.5, 0.5, 0.5}, {0, 1, 0} },
        { {-0.5, 0.5, -0.5}, {0, 1, 0} },
        { {0.5, 0.5, -0.5}, {0, 1, 0} },
        { {-0.5, -0.5, 0.5}, {-1, 0, 0} },
        { {-0.5, 0.5, 0.5}, {-1, 0, 0} },
        { {-0.5, -0.5, -0.5}, {-1, 0, 0} },
        { {-0.5, 0.5, -0.5}, {-1, 0, 0} },
        { {-0.5, -0.5, -0.5}, {0, 0, -1} },
        { {-0.5, 0.5, -0.5}, {0, 0, -1} },
        { {0.5, -0.5, -0.5}, {0, 0, -1} },
        { {0.5, 0.5, -0.5}, {0, 0, -1} },
    };

    uint16_t boxIndices[] = {
        0, 1, 2,
        3, 2, 1,
        4, 5, 6,
        7, 6, 5,
        8, 9, 10,
        11, 10, 9,
        12, 13, 14,
        15, 14, 13,
        16, 17, 18,
        19, 18, 17,
        20, 21, 22,
        23, 22, 21,
    };

    constexpr size_t OBJECT_DATA_CAPACITY = 1024;
}

DebugDrawService::DebugDrawService(gfx::renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(debugDrawScopeId);

    m_vertexBuffer = renderer.create_buffer({
        .size = sizeof(boxVertices),
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::device_local,
        .is_mutable = false,
    });
    renderer.buffer_upload_sync(m_vertexBuffer, { &boxVertices, sizeof(boxVertices) });

    m_indexBuffer = renderer.create_buffer({
        .size = sizeof(boxIndices),
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::device_local,
        .is_mutable = false,
    });
    renderer.buffer_upload_sync(m_indexBuffer, { &boxIndices, sizeof(boxIndices) });

    m_objectBuffer = renderer.create_buffer({
        .size = OBJECT_DATA_CAPACITY * sizeof(ObjectData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    m_objectDescriptorGroup = renderer.create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_objectBuffer}},
        }
    });

    {
        ShaderPackage package{ "data/shaders/packaged/debugdraw.vert" };
        nstl::string const* path = package.get({});
        assert(path);

        m_vertexShader = renderer.create_shader({
            .filename = *path,
            .stage = gfx::shader_stage::vertex,
        });
    }

    {
        ShaderPackage package{ "data/shaders/packaged/debugdraw.frag" };
        nstl::string const* path = package.get({});
        assert(path);

        m_fragmentShader = renderer.create_shader({
            .filename = *path,
            .stage = gfx::shader_stage::fragment,
        });
    }

    // TODO have its own renderpass
    m_renderstate = renderer.create_renderstate({
        .shaders = nstl::array{ m_vertexShader, m_fragmentShader },
        .renderpass = renderer.get_main_renderpass(),
        .vertex_config = {
            .buffer_bindings = nstl::array{
                gfx::buffer_binding_description{ .buffer_index = 0, .stride = 24 }
            },
            .attributes = nstl::array{
                gfx::attribute_description{
                    .location = 0,
                    .buffer_binding_index = 0,
                    .offset = offsetof(BoxVertex, position),
                    .type = gfx::attribute_type::vec3f,
                },
                gfx::attribute_description{
                    .location = 1,
                    .buffer_binding_index = 0,
                    .offset = offsetof(BoxVertex, normal),
                    .type = gfx::attribute_type::vec3f,
                },
            },
            .topology = gfx::vertex_topology::triangles,
        },
        .descriptorgroup_layouts = nstl::array{
            // TODO get the first descriptorgroup layout from the outside
            gfx::descriptorgroup_layout_view{
                .entries = nstl::array{
                    gfx::descriptor_layout_entry{ 0, gfx::descriptor_type::uniform_buffer },
                    gfx::descriptor_layout_entry{ 1, gfx::descriptor_type::uniform_buffer },
                },
            },
            gfx::descriptorgroup_layout_view{
                .entries = nstl::array{
                    gfx::descriptor_layout_entry{
                        .location = 0,
                        .type = gfx::descriptor_type::uniform_buffer,
                    }
                },
            },
        },
        .flags = {
            .cull_backfaces = false,
            .wireframe = true,
            .depth_test = true,
            .alpha_blending = false,
            .depth_bias = false,
        },
    });
}

DebugDrawService::~DebugDrawService() = default;

void DebugDrawService::box(tglm::vec3 const& center, tglm::quat const& rotation, tglm::vec3 const& scale, tglm::vec3 const&, float)
{
    // TODO use color parameter and duration

    MEMORY_TRACKING_SCOPE(debugDrawScopeId);

    assert(m_objectData.size() < OBJECT_DATA_CAPACITY);
    ObjectData& data = m_objectData.emplace_back();

    data.model = tglm::mat4::identity();
    tglm::translate(data.model, center);
    tglm::rotate(data.model, rotation);
    tglm::scale(data.model, scale);
}

void DebugDrawService::updateResources(gfx::renderer& renderer)
{
    renderer.buffer_upload_sync(m_objectBuffer, { m_objectData.data(), m_objectData.size() * sizeof(ObjectData) });

    m_objectData.clear();
}

void DebugDrawService::draw(gfx::renderer& renderer, gfx::descriptorgroup_handle cameraDescriptorGroup)
{
    renderer.draw_indexed({
        .renderstate = m_renderstate,
        .descriptorgroups = nstl::array{ cameraDescriptorGroup, m_objectDescriptorGroup },

        .vertex_buffers = nstl::array{ gfx::buffer_with_offset{ m_vertexBuffer } },
        .index_buffer = { m_indexBuffer },
        .index_type = gfx::index_type::uint16,

        .index_count = sizeof(boxIndices) / sizeof(boxIndices[0]),
    });
}

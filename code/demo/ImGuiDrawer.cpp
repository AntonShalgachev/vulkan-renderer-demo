#include "ImGuiDrawer.h"

// TODO think about these dependencies
#include "ShaderPackage.h"

#include "gfx/renderer.h"

#include "tglm/types.h"
#include "tglm/util.h"

#include "nstl/array.h"
#include "nstl/vector.h"

#include "memory/tracking.h"

#include "imgui.h"

namespace
{
    auto imguiDrawerScopeId = memory::tracking::create_scope_id("UI/ImGui/Drawer");

    // TODO move somewhere
    auto descriptorGroupToTextureId(gfx::descriptorgroup_handle handle)
    {
        ImTextureID textureId;
        static_assert(sizeof(handle) <= sizeof(textureId));
        memcpy(&textureId, &handle, sizeof(handle));

        return textureId;
    };

    // TODO move somewhere
    auto textureIdToDescriptorGroup(ImTextureID textureId)
    {
        gfx::descriptorgroup_handle handle;

        static_assert(sizeof(handle) <= sizeof(textureId));
        memcpy(&handle, &textureId, sizeof(handle));

        return handle;
    };

    struct TransformParams
    {
        ImVec2 scale;
        ImVec2 translate;
    };

    struct ClipData
    {
        tglm::ivec2 clipMin;
        tglm::ivec2 clipMax;
    };

    ClipData calculateClip(ImDrawData const* drawData, ImDrawCmd const* drawCommand)
    {
        tglm::vec2 displayPos = { drawData->DisplayPos.x, drawData->DisplayPos.y };
        tglm::vec2 displaySize = { drawData->DisplaySize.x, drawData->DisplaySize.y };
        tglm::vec2 scale = { drawData->FramebufferScale.x, drawData->FramebufferScale.y };

        tglm::vec2 clipRectMin = { drawCommand->ClipRect.x, drawCommand->ClipRect.y };
        tglm::vec2 clipRectMax = { drawCommand->ClipRect.z, drawCommand->ClipRect.w };

        tglm::vec2 framebufferSize = displaySize * scale;

        tglm::vec2 clipMin = (clipRectMin - displayPos) * scale;
        tglm::vec2 clipMax = (clipRectMax - displayPos) * scale;

        clipMin.x = tglm::clamp(clipMin.x, 0.0f, framebufferSize.x);
        clipMin.y = tglm::clamp(clipMin.y, 0.0f, framebufferSize.y);

        return { static_cast<tglm::ivec2>(clipMin), static_cast<tglm::ivec2>(clipMax) };
    }

    constexpr size_t VERTEX_BUFFER_CAPACITY = 64 * 1024 * sizeof(ImDrawVert);
    constexpr size_t INDEX_BUFFER_CAPACITY = 64 * 1024 * sizeof(ImDrawIdx);
}

ImGuiDrawer::ImGuiDrawer(gfx::renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(imguiDrawerScopeId);

    ImGuiIO& io = ImGui::GetIO();

    io.BackendRendererUserData = nullptr;
    io.BackendRendererName = "vulkan_renderer_demo";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    createBuffers(renderer);
    createImages(renderer);
    createDescriptors(renderer);
    createShaders(renderer);
    createPipeline(renderer);
}

void ImGuiDrawer::updateResources(gfx::renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(imguiDrawerScopeId);

    if (!ImGui::GetCurrentContext())
        return;

    ImGui::Render();

    ImDrawData const* drawData = ImGui::GetDrawData();
    if (!drawData)
        return;

    assert(drawData->Valid);

    uploadBuffers(renderer, drawData);
}

void ImGuiDrawer::draw(gfx::renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(imguiDrawerScopeId);

    if (!ImGui::GetCurrentContext())
        return;

    ImDrawData const* drawData = ImGui::GetDrawData();
    if (!drawData)
        return;

    assert(drawData->Valid);

    size_t vertexOffset = 0;
    size_t indexOffset = 0;
    for (int listIndex = 0; listIndex < drawData->CmdListsCount; listIndex++)
    {
        ImDrawList const* cmdList = drawData->CmdLists[listIndex];

        for (int commandIndex = 0; commandIndex < cmdList->CmdBuffer.Size; commandIndex++)
        {
            const ImDrawCmd* drawCommand = &cmdList->CmdBuffer[commandIndex];
            if (drawCommand->UserCallback)
            {
                // TODO implement callbacks
                assert(false);
            }
            else
            {
                auto [clipMin, clipMax] = calculateClip(drawData, drawCommand);
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                renderer.draw_indexed({
                    .renderstate = m_renderstate,
                    .descriptorgroups = nstl::array{ m_transformDescriptorGroup, textureIdToDescriptorGroup(drawCommand->GetTexID()) },
                    .scissor = gfx::rect { clipMin, clipMax - clipMin },

                    .vertex_buffers = nstl::array{ gfx::buffer_with_offset{ m_vertexBuffer } },
                    .index_buffer = { m_indexBuffer },
                    .index_type = gfx::index_type::uint16,

                    .index_count = drawCommand->ElemCount,
                    .first_index = drawCommand->IdxOffset + indexOffset,
                    .vertex_offset = drawCommand->VtxOffset + vertexOffset,
                });
            }
        }

        vertexOffset += static_cast<size_t>(cmdList->VtxBuffer.Size);
        indexOffset += static_cast<size_t>(cmdList->IdxBuffer.Size);
    }
}

void ImGuiDrawer::createBuffers(gfx::renderer& renderer)
{
    m_vertexBuffer = renderer.create_buffer({
        .size = VERTEX_BUFFER_CAPACITY,
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    m_indexBuffer = renderer.create_buffer({
        .size = INDEX_BUFFER_CAPACITY,
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    // TODO use push constants somehow?
    m_transformBuffer = renderer.create_buffer({
        .size = sizeof(TransformParams),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });
}

void ImGuiDrawer::createImages(gfx::renderer& renderer)
{
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    int bytesPerPixel = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel); // TODO use GetTexDataAsAlpha8?

    assert(width > 0);
    assert(height > 0);
    assert(bytesPerPixel > 0);

    m_fontImage = renderer.create_image({
        .width = static_cast<size_t>(width),
        .height = static_cast<size_t>(height),
        .format = gfx::image_format::r8g8b8a8,
        .type = gfx::image_type::color,
        .usage = gfx::image_usage::upload_sampled,
    });

    renderer.image_upload_sync(m_fontImage, { pixels, static_cast<size_t>(width * height * bytesPerPixel) });

    m_imageSampler = renderer.create_sampler({});
}

void ImGuiDrawer::createDescriptors(gfx::renderer& renderer)
{
    m_transformDescriptorGroup = renderer.create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_transformBuffer, gfx::descriptor_type::uniform_buffer}},
        }
    });

    m_fontImageDescriptorGroup = renderer.create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_fontImage, m_imageSampler}},
        }
    });

    ImGui::GetIO().Fonts->SetTexID(descriptorGroupToTextureId(m_fontImageDescriptorGroup));
}

void ImGuiDrawer::createShaders(gfx::renderer& renderer)
{
    {
        ShaderPackage package{ "data/shaders/packaged/imgui.vert" };
        nstl::string const* path = package.get({});
        assert(path);

        m_vertexShader = renderer.create_shader({
            .filename = *path,
            .stage = gfx::shader_stage::vertex,
        });
    }

    {
        ShaderPackage package{ "data/shaders/packaged/imgui.frag" };
        nstl::string const* path = package.get({});
        assert(path);

        m_fragmentShader = renderer.create_shader({
            .filename = *path,
            .stage = gfx::shader_stage::fragment,
        });
    }
}

void ImGuiDrawer::createPipeline(gfx::renderer& renderer)
{
    // TODO have an own renderpass
    m_renderstate = renderer.create_renderstate({
        .shaders = nstl::array{ m_vertexShader, m_fragmentShader },
        .renderpass = renderer.get_main_renderpass(),
        .vertex_config = {
            .buffer_bindings = nstl::array{
                gfx::buffer_binding_description{ .buffer_index = 0, .stride = sizeof(ImDrawVert) }
            },
            .attributes = nstl::array{
                gfx::attribute_description{
                    .location = 0,
                    .buffer_binding_index = 0,
                    .offset = offsetof(ImDrawVert, pos),
                    .type = gfx::attribute_type::vec2f,
                },
                gfx::attribute_description{
                    .location = 1,
                    .buffer_binding_index = 0,
                    .offset = offsetof(ImDrawVert, uv),
                    .type = gfx::attribute_type::vec2f,
                },
                gfx::attribute_description{
                    .location = 2,
                    .buffer_binding_index = 0,
                    .offset = offsetof(ImDrawVert, col),
                    .type = gfx::attribute_type::uint32,
                },
            },
            .topology = gfx::vertex_topology::triangles,
        },
        .descriptorgroup_layouts = nstl::array{
            gfx::descriptorgroup_layout_view{
                .entries = nstl::array{
                    gfx::descriptor_layout_entry{
                        .location = 0,
                        .type = gfx::descriptor_type::uniform_buffer,
                    }
                },
            },
            gfx::descriptorgroup_layout_view{
                .entries = nstl::array{
                    gfx::descriptor_layout_entry{
                        .location = 0,
                        .type = gfx::descriptor_type::combined_image_sampler,
                    },
                },
            },
        },
        .flags = {
            .cull_backfaces = false,
            .wireframe = false,
            .depth_test = false,
            .alpha_blending = true,
            .depth_bias = false,
        },
    });
}

void ImGuiDrawer::uploadBuffers(gfx::renderer& renderer, ImDrawData const* drawData)
{
    if (drawData->TotalVtxCount <= 0)
        return;

    [[maybe_unused]] size_t vertexBufferSize = static_cast<size_t>(drawData->TotalVtxCount) * sizeof(ImDrawVert);
    [[maybe_unused]] size_t indexBufferSize = static_cast<size_t>(drawData->TotalIdxCount) * sizeof(ImDrawIdx);

    assert(vertexBufferSize <= VERTEX_BUFFER_CAPACITY);
    assert(indexBufferSize <= INDEX_BUFFER_CAPACITY);

    size_t nextVertexBufferOffset = 0;
    size_t nextIndexBufferOffset = 0;

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        auto vertexChunkSize = static_cast<size_t>(cmdList->VtxBuffer.Size) * sizeof(ImDrawVert);
        auto indexChunkSize = static_cast<size_t>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx);

        renderer.buffer_upload_sync(m_vertexBuffer, { cmdList->VtxBuffer.Data, vertexChunkSize }, nextVertexBufferOffset);
        renderer.buffer_upload_sync(m_indexBuffer, { cmdList->IdxBuffer.Data, indexChunkSize }, nextIndexBufferOffset);

        nextVertexBufferOffset += vertexChunkSize;
        nextIndexBufferOffset += indexChunkSize;
    }

    {
        TransformParams transform{
            .scale = {
                2.0f / drawData->DisplaySize.x,
                2.0f / drawData->DisplaySize.y,
            },
            .translate = {
                -1.0f - drawData->DisplayPos.x * transform.scale.x,
                -1.0f - drawData->DisplayPos.y * transform.scale.y,
            },
        };

        renderer.buffer_upload_sync(m_transformBuffer, { &transform, sizeof(transform) });
    }
}

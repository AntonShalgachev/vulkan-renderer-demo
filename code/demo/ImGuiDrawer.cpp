#include "ImGuiDrawer.h"

// TODO think about these dependencies
#include "ShaderPackage.h"

#include "tglm/types.h"
#include "tglm/util.h"

#include "vko/SamplerProperties.h"
#include "vko/ShaderModuleProperties.h"

#include "vkgfx/Renderer.h"
#include "vkgfx/ResourceManager.h"
#include "vkgfx/Handles.h"
#include "vkgfx/ImageMetadata.h"
#include "vkgfx/Texture.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/Material.h"
#include "vkgfx/BufferMetadata.h"
#include "vkgfx/TestObject.h"
#include "vkgfx/PipelineKey.h"

#include "common/Utils.h"

#include "nstl/vector.h"

#include "memory/tracking.h"

#include "imgui.h"

namespace
{
    auto scopeId = memory::tracking::create_scope_id("UI/ImGui/Drawer");

    // TODO move somewhere
    auto imageToTextureId(vkgfx::ImageHandle image)
    {
        ImTextureID textureId;
        static_assert(sizeof(image) <= sizeof(textureId));
        memcpy(&textureId, &image, sizeof(image));

        return textureId;
    };

    // TODO move somewhere
    auto textureIdToImage(ImTextureID textureId)
    {
        vkgfx::ImageHandle image;

        static_assert(sizeof(image) <= sizeof(textureId));
        memcpy(&image, &textureId, sizeof(image));

        return image;
    };

    nstl::static_vector<unsigned char, vkgfx::MaxPushConstantsSize> createPushConstants(ImDrawData const* drawData)
    {
        struct PushConstants
        {
            ImVec2 scale;
            ImVec2 translate;
        };

        PushConstants pushConstants;

        pushConstants.scale = {
            2.0f / drawData->DisplaySize.x,
            2.0f / drawData->DisplaySize.y,
        };
        pushConstants.translate = {
            -1.0f - drawData->DisplayPos.x * pushConstants.scale.x,
            -1.0f - drawData->DisplayPos.y * pushConstants.scale.y,
        };

        nstl::static_vector<unsigned char, vkgfx::MaxPushConstantsSize> bytes;
        bytes.resize(sizeof(PushConstants));
        memcpy(bytes.data(), &pushConstants, sizeof(pushConstants));

        return bytes;
    }

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
}

ImGuiDrawer::ImGuiDrawer(vkgfx::Renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    ImGuiIO& io = ImGui::GetIO();

    io.BackendRendererUserData = nullptr;
    io.BackendRendererName = "vulkan_renderer_demo";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    vkgfx::ResourceManager& resourceManager = renderer.getResourceManager();

    createBuffers(resourceManager);
    createImages(resourceManager);
    createShaders(resourceManager);
    createPipeline(resourceManager);
}

void ImGuiDrawer::queueGeometry(vkgfx::Renderer& renderer)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    if (!ImGui::GetCurrentContext())
        return;

    ImGui::Render();

    ImDrawData const* drawData = ImGui::GetDrawData();
    if (!drawData)
        return;

    assert(drawData->Valid);

    vkgfx::ResourceManager& resourceManager = renderer.getResourceManager();

    uploadBuffers(resourceManager, drawData);

    auto pushConstantsBytes = createPushConstants(drawData);

    size_t nextResourceIndex = 0;
    size_t vertexOffset = 0;
    size_t indexOffset = 0;
    for (int listIndex = 0; listIndex < drawData->CmdListsCount; listIndex++)
    {
        ImDrawList const* cmdList = drawData->CmdLists[listIndex];

        for (size_t commandIndex = 0; commandIndex < cmdList->CmdBuffer.Size; commandIndex++)
        {
            const ImDrawCmd* drawCommand = &cmdList->CmdBuffer[commandIndex];
            if (drawCommand->UserCallback != NULL)
            {
                // TODO implement callbacks
                assert(false);
            }
            else
            {
                auto [clipMin, clipMax] = calculateClip(drawData, drawCommand);
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                updateMesh(resourceManager, nextResourceIndex, drawCommand->ElemCount, drawCommand->IdxOffset + indexOffset, drawCommand->VtxOffset + vertexOffset);
                updateMaterial(resourceManager, nextResourceIndex, textureIdToImage(drawCommand->GetTexID()));

                vkgfx::TestObject object;
                object.pipeline = m_pipeline;
                object.mesh = m_meshes[nextResourceIndex];
                object.material = m_materials[nextResourceIndex];
                object.pushConstants = pushConstantsBytes;

                object.hasScissors = true;
                object.scissorOffset = clipMin;
                object.scissorSize = clipMax - clipMin;

                renderer.addOneFrameTestObject(nstl::move(object));

                nextResourceIndex++;
            }
        }

        vertexOffset += cmdList->VtxBuffer.Size;
        indexOffset += cmdList->IdxBuffer.Size;
    }
}

void ImGuiDrawer::createBuffers(vkgfx::ResourceManager& resourceManager)
{
    {
        size_t size = 64 * 1024 * sizeof(ImDrawVert);
        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::VertexIndexBuffer,
            .location = vkgfx::BufferLocation::HostVisible,
            .isMutable = true,
        };
        m_vertexBuffer = resourceManager.createBuffer(size, nstl::move(metadata));
    }

    {
        size_t size = 64 * 1024 * sizeof(ImDrawIdx);
        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::VertexIndexBuffer,
            .location = vkgfx::BufferLocation::HostVisible,
            .isMutable = true,
        };
        m_indexBuffer = resourceManager.createBuffer(size, nstl::move(metadata));
    }
}

void ImGuiDrawer::createImages(vkgfx::ResourceManager& resourceManager)
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

    vkgfx::ImageMetadata metadata{
        .width = static_cast<size_t>(width),
        .height = static_cast<size_t>(height),
        .byteSize = static_cast<size_t>(width * height * bytesPerPixel),
        .format = vkgfx::ImageFormat::R8G8B8A8,
    };
    m_fontImage = resourceManager.createImage(nstl::move(metadata));

    resourceManager.uploadImage(m_fontImage, pixels, width * height * bytesPerPixel);

    io.Fonts->SetTexID(imageToTextureId(m_fontImage));

    m_imageSampler = resourceManager.createSampler(vko::SamplerFilterMode::Linear, vko::SamplerFilterMode::Linear, vko::SamplerWrapMode::Repeat, vko::SamplerWrapMode::Repeat);
}

void ImGuiDrawer::createShaders(vkgfx::ResourceManager& resourceManager)
{
    {
        ShaderPackage package{ "data/shaders/packaged/imgui.vert" };
        nstl::string const* path = package.get({});
        assert(path);
        if (path)
            m_vertexShaderModule = resourceManager.createShaderModule(vkc::utils::readBinaryFile(*path), vko::ShaderModuleType::Vertex, "main");
    }

    {
        ShaderPackage package{ "data/shaders/packaged/imgui.frag" };
        nstl::string const* path = package.get({});
        assert(path);
        if (path)
            m_fragmentShaderModule = resourceManager.createShaderModule(vkc::utils::readBinaryFile(*path), vko::ShaderModuleType::Fragment, "main");
    }
}

void ImGuiDrawer::createPipeline(vkgfx::ResourceManager& resourceManager)
{
    vkgfx::PipelineKey key;
    key.shaderHandles = { m_vertexShaderModule, m_fragmentShaderModule };
    key.uniformConfigs = {
        // TODO remove unnecessary configs
        vkgfx::UniformConfiguration{
            .hasBuffer = true,
            .hasAlbedoTexture = false,
            .hasNormalMap = false,
        },
        vkgfx::UniformConfiguration{
            .hasBuffer = false,
            .hasAlbedoTexture = true,
            .hasNormalMap = false,
        },
        vkgfx::UniformConfiguration{
            .hasBuffer = false,
            .hasAlbedoTexture = false,
            .hasNormalMap = false,
        },
    };
    key.vertexConfig = {
        .bindings = {
            vkgfx::VertexConfiguration::Binding{
                .stride = sizeof(ImDrawVert),
            },
        },
        .attributes = {
            vkgfx::VertexConfiguration::Attribute{
                .binding = 0,
                .location = 0,
                .offset = offsetof(ImDrawVert, pos),
                .type = vkgfx::AttributeType::Vec2f,
            },
            vkgfx::VertexConfiguration::Attribute{
                .binding = 0,
                .location = 1,
                .offset = offsetof(ImDrawVert, uv),
                .type = vkgfx::AttributeType::Vec2f,
            },
            vkgfx::VertexConfiguration::Attribute{
                .binding = 0,
                .location = 2,
                .offset = offsetof(ImDrawVert, col),
                .type = vkgfx::AttributeType::UInt32,
            },
        },
        .topology = vkgfx::VertexTopology::Triangles,
    };
    key.renderConfig = {
        .cullBackfaces = false,
        .wireframe = false,
        .depthTest = false,
        .alphaBlending = true,
    };
    key.pushConstantRanges = {
        vkgfx::PushConstantRange{
            .offset = 0,
            .size = 2 * sizeof(float) + 2 * sizeof(float),
        }
    };

    m_pipeline = resourceManager.getOrCreatePipeline(key);
}

void ImGuiDrawer::uploadBuffers(vkgfx::ResourceManager& resourceManager, ImDrawData const* drawData)
{
    if (drawData->TotalVtxCount <= 0)
        return;

    size_t vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
    size_t indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

    assert(vertexBufferSize <= resourceManager.getBufferSize(m_vertexBuffer));
    assert(indexBufferSize <= resourceManager.getBufferSize(m_indexBuffer));

    size_t nextVertexBufferOffset = 0;
    size_t nextIndexBufferOffset = 0;

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        auto vertexChunkSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
        auto indexChunkSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

        resourceManager.uploadDynamicBufferToStaging(m_vertexBuffer, cmdList->VtxBuffer.Data, vertexChunkSize, nextVertexBufferOffset);
        resourceManager.uploadDynamicBufferToStaging(m_indexBuffer, cmdList->IdxBuffer.Data, indexChunkSize, nextIndexBufferOffset);

        nextVertexBufferOffset += vertexChunkSize;
        nextIndexBufferOffset += indexChunkSize;
    }
}

void ImGuiDrawer::updateMesh(vkgfx::ResourceManager& resourceManager, size_t index, size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
    static_assert(sizeof(ImDrawIdx) == 2);

    vkgfx::MeshHandle handle;
    if (index < m_meshes.size())
    {
        handle = m_meshes[index];
    }
    else
    {
        assert(index == m_meshes.size());
        handle = resourceManager.createMesh({});
        m_meshes.push_back(handle);
    }

    vkgfx::Mesh* mesh = resourceManager.getMesh(handle);
    assert(mesh);

    mesh->vertexBuffers.resize(1);
    mesh->vertexBuffers[0] = { m_vertexBuffer, 0 };

    mesh->indexBuffer = { m_indexBuffer, 0 };
    mesh->indexCount = indexCount;
    mesh->indexType = vkgfx::IndexType::UnsignedShort;
    mesh->indexOffset = indexOffset;
    mesh->vertexOffset = vertexOffset;
}

void ImGuiDrawer::updateMaterial(vkgfx::ResourceManager& resourceManager, size_t index, vkgfx::ImageHandle image)
{
    vkgfx::Texture texture = {
        .image = image,
        .sampler = m_imageSampler,
    };

    if (index < m_textures.size())
    {
        resourceManager.updateTexture(m_textures[index], nstl::move(texture));
    }
    else
    {
        m_textures.push_back(resourceManager.createTexture(nstl::move(texture)));
    }

    vkgfx::Material material = {
        .albedo = m_textures[index],
    };

    if (index < m_materials.size())
    {
        resourceManager.updateMaterial(m_materials[index], nstl::move(material));
    }
    else
    {
        m_materials.push_back(resourceManager.createMaterial(nstl::move(material)));
    }
}

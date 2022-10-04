#pragma once

#include "vkgfx/Handles.h"

#include "glm.h"

#include <vector>
#include <tuple>

namespace vkgfx
{
    class Renderer;
    class ResourceManager;
}

struct ImDrawData;
struct ImDrawCmd;

class ImGuiDrawer
{
public:
    ImGuiDrawer(vkgfx::Renderer& renderer);

    void draw(vkgfx::Renderer& renderer);

private:
    void createBuffers(vkgfx::ResourceManager& resourceManager);
    void createImages(vkgfx::ResourceManager& resourceManager);
    void createShaders(vkgfx::ResourceManager& resourceManager);
    void createPipeline(vkgfx::ResourceManager& resourceManager);

    void uploadBuffers(vkgfx::ResourceManager& resourceManager, ImDrawData const* drawData);
    std::vector<unsigned char> createPushConstants(ImDrawData const* drawData);
    std::tuple<glm::ivec2, glm::ivec2> calculateClip(ImDrawData const* drawData, ImDrawCmd const* drawCommand);
    void updateMesh(vkgfx::ResourceManager& resourceManager, std::size_t index, std::size_t indexCount, std::size_t indexOffset, std::size_t vertexOffset);
    void updateMaterial(vkgfx::ResourceManager& resourceManager, std::size_t index, vkgfx::ImageHandle image);

private:
    vkgfx::BufferHandle m_vertexBuffer;
    vkgfx::BufferHandle m_indexBuffer;
    vkgfx::ImageHandle m_fontImage;
    vkgfx::SamplerHandle m_imageSampler;
    vkgfx::ShaderModuleHandle m_vertexShaderModule;
    vkgfx::ShaderModuleHandle m_fragmentShaderModule;
    vkgfx::PipelineHandle m_pipeline;
    std::vector<vkgfx::MeshHandle> m_meshes;
    std::vector<vkgfx::TextureHandle> m_textures;
    std::vector<vkgfx::MaterialHandle> m_materials;
};

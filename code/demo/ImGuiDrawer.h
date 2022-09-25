#pragma once

#include "vkgfx/Handles.h"

#include "glm.h"

#include <vector>
#include <tuple>

namespace vkgfx
{
    class Renderer;
}

struct ImDrawData;
struct ImDrawCmd;

class ImGuiDrawer
{
public:
    ImGuiDrawer(vkgfx::Renderer& renderer);

    void draw();

private:
    void createBuffers();
    void createImages();
    void createShaders();
    void createPipeline();

    void uploadBuffers(ImDrawData const* drawData);
    std::vector<unsigned char> createPushConstants(ImDrawData const* drawData);
    std::tuple<glm::ivec2, glm::ivec2> calculateClip(ImDrawData const* drawData, ImDrawCmd const* drawCommand);
    void updateMesh(std::size_t index, std::size_t indexCount, std::size_t indexOffset, std::size_t vertexOffset);
    void updateMaterial(std::size_t index, vkgfx::ImageHandle image);

private:
    vkgfx::Renderer& m_renderer;

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

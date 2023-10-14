#pragma once

#include "gfx/resources.h"

struct ImDrawData;

namespace gfx
{
    class renderer;
}

class ImGuiDrawer
{
public:
    ImGuiDrawer(gfx::renderer& renderer);

    void updateResources(gfx::renderer& renderer);
    void draw(gfx::renderer& renderer);

private:
    void createBuffers(gfx::renderer& renderer);
    void createImages(gfx::renderer& renderer);
    void createDescriptors(gfx::renderer& renderer);
    void createShaders(gfx::renderer& renderer);
    void createPipeline(gfx::renderer& renderer);

    void uploadBuffers(gfx::renderer& renderer, ImDrawData const* drawData);

private:
    gfx::buffer_handle m_vertexBuffer;
    gfx::buffer_handle m_indexBuffer;

    gfx::buffer_handle m_transformBuffer;
    gfx::image_handle m_fontImage;
    gfx::sampler_handle m_imageSampler;

    gfx::descriptorgroup_handle m_transformDescriptorGroup;
    gfx::descriptorgroup_handle m_fontImageDescriptorGroup;

    gfx::shader_handle m_vertexShader;
    gfx::shader_handle m_fragmentShader;
    gfx::renderstate_handle m_renderstate;
};

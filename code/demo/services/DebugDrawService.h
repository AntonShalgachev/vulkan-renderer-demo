#pragma once

#include "ServiceContainer.h"

#include "gfx/resources.h"

#include "nstl/vector.h"

#include "tglm/fwd.h"

namespace gfx
{
    class renderer;
}

class DebugDrawService
{
public:
    enum class Mode
    {
        Solid,
        Wireframe,
    };

    enum class ZTest
    {
        Disable,
        Enable,
    };

public:
    DebugDrawService(gfx::renderer& renderer);
    ~DebugDrawService();

    void box(tglm::vec3 const& center, tglm::quat const& rotation, tglm::vec3 const& scale, tglm::vec3 const& color, float duration);

    void beginFrame();
    void updateResources(gfx::renderer& renderer);
    void draw(gfx::renderer& renderer, gfx::descriptorgroup_handle cameraDescriptorGroup);
    void endFrame();

private:
    gfx::buffer_handle m_vertexBuffer;
    gfx::buffer_handle m_indexBuffer;
    gfx::buffer_handle m_objectBuffer;
    gfx::descriptorgroup_handle m_objectDescriptorGroup;
    gfx::shader_handle m_vertexShader;
    gfx::shader_handle m_fragmentShader;
    gfx::renderstate_handle m_renderstate;

    struct ObjectData
    {
        tglm::mat4 model;
    };

    nstl::vector<ObjectData> m_objectData;
    size_t m_instancesCount = 0;
};

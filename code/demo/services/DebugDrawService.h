#pragma once

#include "ServiceContainer.h"

#include "vkgfx/Handles.h"

#include "nstl/vector.h"

#include "tglm/fwd.h"

namespace vkgfx
{
    class Renderer;
    struct TestObject;
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
    DebugDrawService(vkgfx::Renderer& renderer);
    ~DebugDrawService();

    void box(tglm::vec3 const& center, tglm::quat const& rotation, tglm::vec3 const& scale, tglm::vec3 const& color, float duration);

    void queueGeometry(vkgfx::Renderer& renderer);

private:
    vkgfx::BufferHandle m_buffer;
    vkgfx::MeshHandle m_mesh;
    vkgfx::PipelineHandle m_pipeline;
    nstl::vector<vkgfx::TestObject> m_objects;
};

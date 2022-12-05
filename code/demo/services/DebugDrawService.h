#pragma once

#include "ServiceContainer.h"

#include "vkgfx/Handles.h"

#include "nstl/vector.h"

#include "common/glm-fwd.h"

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

    void sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration);
    void box(glm::vec3 const& center, glm::quat const& rotation, glm::vec3 const& scale, glm::vec3 const& color, float duration);

    void queueGeometry(vkgfx::Renderer& renderer);

private:
    vkgfx::BufferHandle m_buffer;
    vkgfx::MeshHandle m_mesh;
    vkgfx::PipelineHandle m_pipeline;
    nstl::vector<vkgfx::TestObject> m_objects;
};

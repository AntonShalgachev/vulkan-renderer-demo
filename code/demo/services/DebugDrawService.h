#pragma once

#include "ServiceContainer.h"
#include "Renderer.h"

#include "vkgfx/Handles.h"
#include "vkgfx/TestObject.h"

#include "glm.h"

#include <vector>

namespace vkgfx
{
    class Renderer;
}

class DebugDrawService : public ServiceContainer
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
    DebugDrawService(Services& services);
    ~DebugDrawService();

    void init(vkgfx::Renderer& renderer); // TODO move to the constructor

    void sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration);
    void box(glm::vec3 const& center, glm::quat const& rotation, glm::vec3 const& scale, glm::vec3 const& color, float duration);

    void draw(vkr::Renderer& renderer);
    void draw(vkgfx::Renderer& renderer);

private:
    std::vector<vkr::OneFrameBoxInstance> m_instances;

    vkgfx::BufferHandle m_buffer;
    vkgfx::MeshHandle m_mesh;
    vkgfx::PipelineHandle m_pipeline;
    std::vector<vkgfx::TestObject> m_objects;
};

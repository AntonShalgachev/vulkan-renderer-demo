#pragma once

#include "ServiceContainer.h"
#include "Renderer.h"

#include "glm.h"

#include <vector>

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

    void sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration);
    void box(glm::vec3 const& center, glm::quat const& rotation, glm::vec3 const& scale, glm::vec3 const& color, float duration);

    void draw(vkr::Renderer& renderer);

private:
    std::vector<vkr::OneFrameBoxInstance> m_instances;
};

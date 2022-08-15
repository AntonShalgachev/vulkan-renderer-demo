#pragma once

#include "ServiceContainer.h"

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

    void sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration);
    void box(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration);

private:
    enum class GeometryType
    {
        Sphere,
        Box,
    };

    struct Instance
    {
        GeometryType type;
        glm::mat4 transform;
        glm::vec3 color;
    };

    std::vector<Instance> m_instances;
};

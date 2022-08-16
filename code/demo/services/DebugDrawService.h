#pragma once

#include "ServiceContainer.h"

#include "glm.h"

#include <memory>

namespace vkr
{
    class Drawable;
    class Mesh;
    class Transform;
    class Application;
    class BufferWithMemory;
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
    DebugDrawService(vkr::Application const& app, Services& services);
    ~DebugDrawService();

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
        std::unique_ptr<vkr::Drawable> drawable;
        std::unique_ptr<vkr::Transform> transform; // TODO could be allocated on the stack
    };

    struct PrimitiveResources
    {
        std::unique_ptr<vkr::BufferWithMemory> bufferWithMemory;
        std::shared_ptr<vkr::Mesh> mesh;
        std::unique_ptr<vkr::Drawable> drawable;
    };

private:
    vkr::Application const& m_app;
    PrimitiveResources m_boxResources;

    std::vector<Instance> m_instances;
};

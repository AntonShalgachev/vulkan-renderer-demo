#include "DebugDrawService.h"

#include <vector>

DebugDrawService::DebugDrawService(Services& services) : ServiceContainer(services)
{

}

DebugDrawService::~DebugDrawService() = default;

void DebugDrawService::sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration)
{

}

void DebugDrawService::box(glm::vec3 const& center, glm::quat const& rotation, glm::vec3 const& scale, glm::vec3 const& color, float duration)
{
    auto matrix = glm::identity<glm::mat4>();

    matrix = glm::translate(matrix, center);
    matrix = matrix * glm::mat4_cast(rotation);
    matrix = glm::scale(matrix, scale);

    m_instances.push_back(vkr::OneFrameBoxInstance{ matrix, color });
}

void DebugDrawService::draw(vkr::Renderer& renderer)
{
    renderer.setOneFrameBoxes(m_instances);
    m_instances.clear();
}

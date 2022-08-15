#include "DebugDrawService.h"

DebugDrawService::DebugDrawService(Services& services) : ServiceContainer(services)
{
    // create mesh data
    // create vertex layout

    // TOOD remove
    box({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, -1.0f);
}

void DebugDrawService::sphere(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration)
{

}

void DebugDrawService::box(glm::vec3 const& center, glm::vec3 const& scale, glm::vec3 const& color, float duration)
{

}

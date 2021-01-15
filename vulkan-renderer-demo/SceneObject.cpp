#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void vkr::SceneObject::setMesh(std::shared_ptr<Mesh> const& mesh)
{
    m_mesh = mesh;
}

void vkr::SceneObject::setMaterial(std::shared_ptr<Material> const& material)
{
    m_material = material;
}

glm::mat4 vkr::SceneObject::getMatrix() const
{
    return glm::translate(glm::mat4(1.0f), m_pos) * glm::toMat4(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);
}

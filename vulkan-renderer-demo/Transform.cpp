#include "Transform.h"

vkr::Transform::Transform(glm::vec3 const& pos, glm::quat const& rotation, glm::vec3 const& scale)
    : m_pos(pos)
    , m_rotation(rotation)
    , m_scale(scale)
{

}

glm::vec3 vkr::Transform::transformPointLocalToWorld(glm::vec3 const& point) const
{
    return (getMatrix() * glm::vec4(point, 1.0f)).xyz();
}

glm::vec3 vkr::Transform::transformPointWorldToLocal(glm::vec3 const& point) const
{
    return (getInverseMatrix() * glm::vec4(point, 1.0f)).xyz();
}

glm::vec3 vkr::Transform::transformVectorLocalToWorld(glm::vec3 const& vector) const
{
    return (getMatrix() * glm::vec4(vector, 0.0f)).xyz();
}

glm::vec3 vkr::Transform::transformVectorWorldToLocal(glm::vec3 const& vector) const
{
    return (getInverseMatrix() * glm::vec4(vector, 0.0f)).xyz();
}

glm::vec3 vkr::Transform::getRightVector() const
{
    return transformVectorLocalToWorld(glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 vkr::Transform::getRightPoint() const
{
    return transformPointLocalToWorld(glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 vkr::Transform::getForwardVector() const
{
    return transformVectorLocalToWorld(glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 vkr::Transform::getForwardPoint() const
{
    return transformPointLocalToWorld(glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 vkr::Transform::getUpVector() const
{
    return transformVectorLocalToWorld(glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::vec3 vkr::Transform::getUpPoint() const
{
    return transformPointLocalToWorld(glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 vkr::Transform::getMatrix() const
{
    // TODO cache
    return glm::translate(glm::mat4(1.0f), m_pos) * glm::toMat4(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);
}

glm::mat4 vkr::Transform::getInverseMatrix() const
{
    // TODO cache
    return glm::inverse(getMatrix());
}

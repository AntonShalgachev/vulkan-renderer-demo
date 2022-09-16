#include "Transform.h"
#include <stdexcept>

vkr::Transform::Transform(glm::vec3 const& pos, glm::quat const& rotation, glm::vec3 const& scale)
    : m_pos(pos)
    , m_rotation(rotation)
    , m_scale(scale)
{

}

void vkr::Transform::setLocalMatrix(glm::mat4 const& matrix)
{
	glm::vec3 skew;
	glm::vec4 perspective;
    glm::decompose(matrix, m_scale, m_rotation, m_pos, skew, perspective);
    onLocalMatrixChanged();
}

void vkr::Transform::addChild(Transform& child)
{
    if (child.m_parent != nullptr)
        throw std::runtime_error("child already has a parent");

    child.m_parent = this;
	m_children.emplace_back(child);

    notifyChildrenOnParentMatrixInvalidated();
}

glm::vec3 vkr::Transform::getWorldPos() const
{
    return transformPointLocalToWorld(glm::vec3(0.0f, 0.0f, 0.0f));
}

glm::quat vkr::Transform::getWorldRotation() const
{
    return glm::toQuat(getMatrix());
}

void vkr::Transform::setWorldPos(glm::vec3 const& worldPos)
{
    auto localPos = worldPos;
    if (m_parent)
        localPos = m_parent->transformPointWorldToLocal(worldPos);

    setLocalPos(localPos);
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
    return glm::normalize(transformVectorLocalToWorld(glm::vec3(1.0f, 0.0f, 0.0f)));
}

glm::vec3 vkr::Transform::getRightPoint() const
{
    return transformPointLocalToWorld(glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 vkr::Transform::getForwardVector() const
{
    return glm::normalize(transformVectorLocalToWorld(glm::vec3(0.0f, 0.0f, -1.0f)));
}

glm::vec3 vkr::Transform::getForwardPoint() const
{
    return transformPointLocalToWorld(glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 vkr::Transform::getUpVector() const
{
    return glm::normalize(transformVectorLocalToWorld(glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 vkr::Transform::getUpPoint() const
{
    return transformPointLocalToWorld(glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 vkr::Transform::getMatrix() const
{
    if (!m_cache.matrix)
        m_cache.matrix = getParentMatrix() * getLocalMatrix();

    return *m_cache.matrix;
}

glm::mat4 vkr::Transform::getParentMatrix() const
{
    if (!m_cache.parentMatrix)
    {
		glm::mat4 parentMatrix = glm::identity<glm::mat4>();
		if (m_parent)
			parentMatrix = m_parent->getMatrix();

        m_cache.parentMatrix = parentMatrix;
    }

    return *m_cache.parentMatrix;
}

glm::mat4 vkr::Transform::getLocalMatrix() const
{
    if (!m_cache.localMatrix)
        m_cache.localMatrix = glm::translate(glm::mat4(1.0f), m_pos) * glm::toMat4(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);

    return *m_cache.localMatrix;
}

glm::mat4 vkr::Transform::getInverseMatrix() const
{
    if (!m_cache.inverseMatrix)
        m_cache.inverseMatrix = glm::inverse(getMatrix());
    
    return *m_cache.inverseMatrix;
}

glm::mat4 vkr::Transform::getViewMatrix() const
{
    return getInverseMatrix();
}

void vkr::Transform::onLocalMatrixChanged()
{
	m_cache.invalidateLocal();

    notifyChildrenOnParentMatrixInvalidated();
}

void vkr::Transform::notifyChildrenOnParentMatrixInvalidated()
{
	for (Transform& child : m_children)
		child.onParentMatrixInvalidated();
}

void vkr::Transform::onParentMatrixInvalidated()
{
    m_cache.invalidateParent();

    notifyChildrenOnParentMatrixInvalidated();
}

#include "Camera.h"

glm::mat4 vkr::Camera::getViewMatrix() const
{
    // TODO cache
    return glm::lookAt(m_transform.getPos(), m_transform.getForwardPoint(), m_transform.getUpVector());
}

glm::mat4 vkr::Camera::getProjectionMatrix() const
{
    // TODO cache
    auto proj = glm::perspective(glm::radians(m_fov), m_aspect, m_nearZ, m_farZ);
    proj[1][1] *= -1;
    return proj;
}

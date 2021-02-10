#include "Camera.h"

glm::mat4 vkr::Camera::getViewProjectionMatrix() const
{
    // TODO cache

    auto view = glm::lookAt(m_transform.getPos(), m_transform.getForwardPoint(), m_transform.getUpVector());
    auto proj = glm::perspective(glm::radians(m_fov), m_aspect, m_nearZ, m_farZ);
    proj[1][1] *= -1;

    return proj * view;
}

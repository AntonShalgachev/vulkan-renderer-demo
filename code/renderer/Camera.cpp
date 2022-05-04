#include "Camera.h"

glm::mat4 vkr::Camera::getProjectionMatrix() const
{
    // TODO cache
    auto proj = glm::perspective(glm::radians(m_fov), m_aspect, m_nearZ, m_farZ);
    proj[1][1] *= -1;
    return proj;
}

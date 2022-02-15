#pragma once

#include "glm.h"

namespace vkr
{
    class Camera
    {
    public:
        void setFov(float fov) { m_fov = fov; }
        void setAspect(float aspect) { m_aspect = aspect; }
        void setPlanes(float nearZ, float farZ) { m_nearZ = nearZ; m_farZ = farZ; }

        glm::mat4 getProjectionMatrix() const;

    private:
        float m_fov = 45.0f;
        float m_aspect = 1.0f;
        float m_nearZ = 0.1f;
        float m_farZ = 10000.0f;
    };
}

#pragma once
#include "Transform.h"

namespace vkr
{
    class Camera
    {
    public:
        Transform& getTransform() { return m_transform; }
        Transform const& getTransform() const { return m_transform; }

        void setFov(float fov) { m_fov = fov; }
        void setAspect(float aspect) { m_aspect = aspect; }
        float setPlanes(float nearZ, float farZ) { m_nearZ = nearZ; m_farZ = farZ; }

        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix() const;

    private:
        Transform m_transform;

        float m_fov = 45.0f;
        float m_aspect = 1.0f;
        float m_nearZ = 0.1f;
        float m_farZ = 10.0f;
    };
}

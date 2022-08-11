#pragma once
#include "Transform.h"

namespace vkr
{
    class Light
    {
    public:
        Transform& getTransform() { return m_transform; }
        Transform const& getTransform() const { return m_transform; }

        glm::vec3 const& getColor() const { return m_color; }
        void setColor(glm::vec3 const& color) { m_color = color; }

        float getIntensity() const { return m_intensity; }
        void setIntensity(float intensity) { m_intensity = intensity; }

    private:
        glm::vec3 m_color = glm::vec3(1.0f, 1.0f, 1.0f);
        float m_intensity = 1.0f;
        Transform m_transform;
    };
}

#pragma once

#include "glm.h"
#include <glm/gtc/quaternion.hpp>

namespace vkr
{
    class Transform
    {
    public:
        Transform(glm::vec3 const& pos = glm::vec3(0.0f), glm::quat const& rotation = glm::identity<glm::quat>(), glm::vec3 const& scale = glm::vec3(1.0f));

        void setLocalPos(glm::vec3 const& pos) { m_pos = pos; }
        void setLocalRotation(glm::quat const& rotation) { m_rotation = rotation; }
        void setLocalScale(glm::vec3 const& scale) { m_scale = scale; }

        void setLocalMatrix(glm::mat4 const& matrix);

        void addChild(Transform& child);
        std::vector<std::reference_wrapper<Transform>> const& getChildren() const { return m_children; }

        glm::vec3 const& getLocalPos() const { return m_pos; }
        glm::quat const& getLocalRotation() const { return m_rotation; }
        glm::vec3 const& getLocalScale() const { return m_scale; }

        glm::vec3 getWorldPos() const;
        glm::quat getWorldRotation() const;

        void setWorldPos(glm::vec3 const& worldPos);

        glm::vec3 transformPointLocalToWorld(glm::vec3 const& point) const;
        glm::vec3 transformPointWorldToLocal(glm::vec3 const& point) const;
        glm::vec3 transformVectorLocalToWorld(glm::vec3 const& vector) const;
        glm::vec3 transformVectorWorldToLocal(glm::vec3 const& vector) const;

        glm::vec3 getRightVector() const;
        glm::vec3 getRightPoint() const;
        glm::vec3 getForwardVector() const;
        glm::vec3 getForwardPoint() const;
        glm::vec3 getUpVector() const;
        glm::vec3 getUpPoint() const;

        glm::mat4 getMatrix() const;
        glm::mat4 getLocalMatrix() const;
        glm::mat4 getInverseMatrix() const;
        glm::mat4 getViewMatrix() const;

    private:
        glm::vec3 m_pos = glm::vec3(0.0f);
        glm::quat m_rotation = glm::identity<glm::quat>();
        glm::vec3 m_scale = glm::vec3(1.0f);

        Transform* m_parent = nullptr;
        std::vector<std::reference_wrapper<Transform>> m_children;
    };
}

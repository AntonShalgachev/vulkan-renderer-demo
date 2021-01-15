#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace vkr
{
    class Mesh;
    class Material;

    class SceneObject
    {
    public:
        void setMesh(std::shared_ptr<Mesh> const& mesh);
        void setMaterial(std::shared_ptr<Material> const& material);

        Mesh const& getMesh() const { return *m_mesh; }
        Material const& getMaterial() const { return *m_material; }

        void setPos(glm::vec3 const& pos) { m_pos = pos; }
        void setRotation(glm::quat const& rotation) { m_rotation = rotation; }
        void setScale(glm::vec3 const& scale) { m_scale = scale; }

        glm::mat4 getMatrix() const;

    private:
        glm::vec3 m_pos = glm::vec3(0.0f);
        glm::quat m_rotation = glm::identity<glm::quat>();
        glm::vec3 m_scale = glm::vec3(1.0f);

        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Material> m_material;
    };
}

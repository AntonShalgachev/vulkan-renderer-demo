#pragma once

#include <memory>
#include "Transform.h"

namespace vkr
{
    class Mesh;
    class Material;
    class Camera;

    class SceneObject
    {
    public:
        void setMesh(std::shared_ptr<Mesh> const& mesh) { m_mesh = mesh; }
        void setMaterial(std::shared_ptr<Material> const& material) { m_material = material; }
        void setCamera(std::shared_ptr<Camera> const& camera) { m_camera = camera; }

        Transform& getTransform() { return m_transform; }
        Transform const& getTransform() const { return m_transform; }

        std::shared_ptr<Mesh> const& getMesh() const { return m_mesh; }
        std::shared_ptr<Material> const& getMaterial() const { return m_material; }
		std::shared_ptr<Camera> const& getCamera() const { return m_camera; }

        bool isDrawable() const { return m_mesh && m_material; }

    private:
        Transform m_transform;

        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Camera> m_camera;
    };
}

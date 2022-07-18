#pragma once

#include <memory>
#include <stdexcept>

namespace vkr
{
    class Mesh;
    class Material;

    class Drawable
    {
    public:
        Drawable(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : m_mesh(std::move(mesh)), m_material(std::move(material))
        {
            if (!m_mesh)
                throw std::runtime_error("Mesh is nullptr");
            if (!m_material)
                throw std::runtime_error("Material is nullptr");
        }

        Mesh const& getMesh() const { return *m_mesh; }
        Material const& getMaterial() const { return *m_material; }

    private:
        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Material> m_material;
    };
}

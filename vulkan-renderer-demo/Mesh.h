#pragma once

#include "framework.h"

namespace vkr
{
    struct Vertex;

    class Mesh
    {
    public:
    	Mesh(std::string const& path);

        std::vector<vkr::Vertex> const& getVertices() { return m_vertices; }
        std::vector<uint32_t> const& getIndices() { return m_indices; }

        std::size_t getVertexCount() const { return m_vertices.size(); }
        std::size_t getIndexCount() const { return m_indices.size(); }

    private:
        std::vector<vkr::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
}

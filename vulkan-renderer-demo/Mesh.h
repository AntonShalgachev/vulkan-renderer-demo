#pragma once

#include "framework.h"

namespace vkr
{
    struct Vertex;
    class Buffer;
    class DeviceMemory;

    class Mesh
    {
    public:
    	Mesh(std::string const& path);

        void bindBuffers(VkCommandBuffer commandBuffer);

        std::vector<vkr::Vertex> const& getVertices() { return m_vertices; }
        std::vector<uint32_t> const& getIndices() { return m_indices; }

        std::size_t getVertexCount() const { return m_vertices.size(); }
        std::size_t getIndexCount() const { return m_indices.size(); }

    private:
        void loadMesh(std::string const& path);
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        std::vector<vkr::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        std::unique_ptr<vkr::Buffer> m_vertexBuffer;
        std::unique_ptr<vkr::DeviceMemory> m_vertexBufferMemory;
        std::unique_ptr<vkr::Buffer> m_indexBuffer;
        std::unique_ptr<vkr::DeviceMemory> m_indexBufferMemory;
    };
}

#include "Mesh.h"

#include <tiny_obj_loader.h>

#include "Vertex.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include "Utils.h"

namespace
{

}

vkr::Mesh::Mesh(Application const& app, std::string const& path) : Object(app)
{
    loadMesh(path);
    createVertexBuffer();
    createIndexBuffer();
}

void vkr::Mesh::bindBuffers(VkCommandBuffer commandBuffer) const
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer->getHandle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);
}

std::size_t vkr::Mesh::getVertexCount() const
{
    return m_vertices.size();
}

void vkr::Mesh::loadMesh(std::string const& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warnings, errors;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warnings, &errors, path.c_str()))
        throw std::runtime_error(warnings + errors);

    std::unordered_map<vkr::Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            std::size_t vertexIndex = static_cast<std::size_t>(index.vertex_index);
            std::size_t texcoordIndex = static_cast<std::size_t>(index.texcoord_index);

            vkr::Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * vertexIndex + 0],
                attrib.vertices[3 * vertexIndex + 1],
                attrib.vertices[3 * vertexIndex + 2],
            };

            vertex.texCoord = {
                attrib.texcoords[2 * texcoordIndex + 0],
                1.0f - attrib.texcoords[2 * texcoordIndex + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void vkr::Mesh::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    std::unique_ptr<vkr::Buffer> stagingBuffer;
    std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    stagingBufferMemory->copyFrom(m_vertices);

    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    vkr::Buffer::copy(*stagingBuffer, *m_vertexBuffer);
}

void vkr::Mesh::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    std::unique_ptr<vkr::Buffer> stagingBuffer;
    std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    stagingBufferMemory->copyFrom(m_indices);

    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    vkr::Buffer::copy(*stagingBuffer, *m_indexBuffer);
}

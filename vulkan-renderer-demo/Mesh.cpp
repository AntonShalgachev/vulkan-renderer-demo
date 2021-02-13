#include "Mesh.h"

#include <tiny_obj_loader.h>

#include "Buffer.h"
#include "DeviceMemory.h"
#include "Utils.h"
#include "tiny_gltf.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };
}

namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const
        {
            auto pos = hash<glm::vec3>()(vertex.pos);
            auto color = hash<glm::vec3>()(vertex.color);
            auto texCoord = hash<glm::vec2>()(vertex.texCoord);

            return ((pos ^ (color << 1)) >> 1) ^ (texCoord << 1);
        }
    };
}

vkr::Mesh const* vkr::Mesh::ms_boundMesh = nullptr;

vkr::Mesh::Mesh(Application const& app, std::string const& path) : Object(app)
{
    loadMesh(path);
    createBuffers(m_combinedData);
}

vkr::Mesh::Mesh(Application const& app, std::shared_ptr<tinygltf::Model> const& model, std::size_t meshIndex, std::size_t primitiveIndex) : Object(app)
{
    m_gltfModel = model;

    tinygltf::Primitive const& primitive = model->meshes[meshIndex].primitives[primitiveIndex];

    for (auto const& entry : primitive.attributes)
    {
        std::string const& name = entry.first;
        int accessorIndex = entry.second;

        tinygltf::Accessor const& accessor = model->accessors[static_cast<std::size_t>(accessorIndex)];
        tinygltf::BufferView const& bufferView = model->bufferViews[static_cast<std::size_t>(accessor.bufferView)];
        tinygltf::Buffer const& buffer = model->buffers[static_cast<std::size_t>(bufferView.buffer)];

        (void*)&buffer;
        (void*)&name;
    }

    std::size_t bufferIndex = 0; // TODO use all buffers
    std::vector<unsigned char> const& data = m_gltfModel->buffers[bufferIndex].data;

    createBuffers(data);
}

void vkr::Mesh::bindBuffers(VkCommandBuffer commandBuffer) const
{
    if (ms_boundMesh == this)
        return;

    VkBuffer vertexBuffers[] = { m_buffer->getHandle(), m_buffer->getHandle() };
    VkDeviceSize offsets[] = { m_positionColorDataOffset, m_texcoordDataOffset };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

    // TODO fix index type
    vkCmdBindIndexBuffer(commandBuffer, m_buffer->getHandle(), m_indexDataOffset, VK_INDEX_TYPE_UINT32);

    ms_boundMesh = this;
}

void vkr::Mesh::loadMesh(std::string const& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warnings, errors;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warnings, &errors, path.c_str()))
        throw std::runtime_error(warnings + errors);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            std::size_t vertexIndex = static_cast<std::size_t>(index.vertex_index);
            std::size_t texcoordIndex = static_cast<std::size_t>(index.texcoord_index);

            Vertex vertex{};

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
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    struct PositionColor
    {
        glm::vec3 pos;
        glm::vec3 color;
    };

    std::vector<PositionColor> positionColors;
    std::vector<glm::vec2> texCoords;

    for (Vertex const& vertex : vertices)
    {
        PositionColor& positionColor = positionColors.emplace_back();
        positionColor.pos = vertex.pos;
        positionColor.color = vertex.color;

        texCoords.push_back(vertex.texCoord);
    }

    m_positionColorBufferSize = sizeof(positionColors[0]) * positionColors.size();
    m_texcoordBufferSize = sizeof(texCoords[0]) * texCoords.size();
    m_indexBufferSize = sizeof(indices[0]) * indices.size();

    m_positionColorDataOffset = 0;
    m_texcoordDataOffset = m_positionColorBufferSize;
    m_indexDataOffset = m_positionColorBufferSize + m_texcoordBufferSize;

    m_indexCount = indices.size();

    m_combinedData.resize(m_positionColorBufferSize + m_texcoordBufferSize + m_indexBufferSize);
    std::memcpy(m_combinedData.data() + m_positionColorDataOffset, positionColors.data(), m_positionColorBufferSize);
    std::memcpy(m_combinedData.data() + m_texcoordDataOffset, texCoords.data(), m_texcoordBufferSize);
    std::memcpy(m_combinedData.data() + m_indexDataOffset, indices.data(), m_indexBufferSize);

    std::vector<VertexLayout::Binding> bindings;
    bindings.emplace_back(sizeof(glm::vec3) + sizeof(glm::vec3))
        .addAttribute(0, VertexLayout::AttributeType::Vec3, VertexLayout::AttributeComponentType::Float, 0)
        .addAttribute(1, VertexLayout::AttributeType::Vec3, VertexLayout::AttributeComponentType::Float, sizeof(glm::vec3));

    bindings.emplace_back(sizeof(glm::vec2))
        .addAttribute(2, VertexLayout::AttributeType::Vec2, VertexLayout::AttributeComponentType::Float, 0);

    m_vertexLayout = VertexLayout{ bindings };
}

void vkr::Mesh::createBuffers(std::vector<unsigned char> const& rawData)
{
    VkDeviceSize bufferSize = sizeof(rawData[0]) * rawData.size();
    void const* bufferData = rawData.data();

    std::unique_ptr<vkr::Buffer> stagingBuffer;
    std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    stagingBufferMemory->copyFrom(bufferData, bufferSize);

    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);

    vkr::Buffer::copy(*stagingBuffer, *m_buffer);
}

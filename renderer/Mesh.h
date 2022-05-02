#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>
#include <string>
#include "VertexLayout.h"

namespace tinygltf
{
    class Model;
    struct Primitive;
}

namespace vkr
{
    class Buffer;
    class DeviceMemory;

    class Mesh : Object
    {
    public:
        struct VertexTraits
        {
            bool hasColor = false;
            bool hasNormal = false;
            bool hasTexCoord = false;
        };

    	Mesh(Application const& app, std::shared_ptr<tinygltf::Model> const& model, tinygltf::Primitive const& primitive);
        ~Mesh();

        VertexLayout const& getVertexLayout() const { return m_vertexLayout; }
        VertexTraits const& getVertexTraits() const { return m_vertexTraits; }

        void bindBuffers(VkCommandBuffer commandBuffer) const;

        std::size_t getIndexCount() const { return m_indexCount; }

    public:
        static void resetBoundMesh() { ms_boundMesh = nullptr; }

    private:
        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        std::vector<unsigned char> m_combinedData;

        std::size_t m_indexCount = 0;

        std::unique_ptr<vkr::Buffer> m_buffer;
        std::unique_ptr<vkr::DeviceMemory> m_bufferMemory;

        std::shared_ptr<tinygltf::Model> m_gltfModel;

        VertexLayout m_vertexLayout;
        VertexTraits m_vertexTraits;

    private:
        static Mesh const* ms_boundMesh;
    };
}

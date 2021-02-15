#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>
#include <string>
#include "VertexLayout.h"

namespace tinygltf { class Model; }

namespace vkr
{
    class Buffer;
    class DeviceMemory;

    class Mesh : Object
    {
    public:
    	Mesh(Application const& app, std::string const& path);
    	Mesh(Application const& app, std::shared_ptr<tinygltf::Model> const& model, std::size_t meshIndex, std::size_t primitiveIndex);

        VertexLayout getVertexLayout() const { return m_vertexLayout; }

        void bindBuffers(VkCommandBuffer commandBuffer) const;

        std::size_t getIndexCount() const { return m_indexCount; }

    public:
        static void resetBoundMesh() { ms_boundMesh = nullptr; }

    private:
        void loadMesh(std::string const& path);

        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        std::vector<unsigned char> m_combinedData;

        std::size_t m_indexCount = 0;

        std::unique_ptr<vkr::Buffer> m_buffer;
        std::unique_ptr<vkr::DeviceMemory> m_bufferMemory;

        std::shared_ptr<tinygltf::Model> m_gltfModel;

        VertexLayout m_vertexLayout;

    private:
        static Mesh const* ms_boundMesh;
    };
}

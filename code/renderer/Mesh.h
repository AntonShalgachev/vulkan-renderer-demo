#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>
#include <string>
#include "VertexLayout.h"

namespace vkr
{
    class Buffer;
    class DeviceMemory;

    class Mesh : Object
    {
    public:
        Mesh(Application const& app, std::vector<unsigned char> const& data, VertexLayout layout);
        ~Mesh();

        VertexLayout const& getVertexLayout() const { return m_vertexLayout; }

        void bindBuffers(VkCommandBuffer commandBuffer) const;

        std::size_t getIndexCount() const { return m_vertexLayout.getIndexCount(); }

    public:
        static void resetBoundMesh() { ms_boundMesh = nullptr; }

    private:
        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        std::unique_ptr<vkr::Buffer> m_buffer;
        std::unique_ptr<vkr::DeviceMemory> m_bufferMemory;

        VertexLayout m_vertexLayout;

    private:
        static Mesh const* ms_boundMesh;
    };
}

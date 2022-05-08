#pragma once

#include "VertexLayout.h"
#include "BufferWithMemory.h"

namespace vkr
{
    class Application;
    class BufferWithMemory;

    class Mesh
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
        vkr::BufferWithMemory m_buffer;
        VertexLayout m_vertexLayout;

    private:
        static Mesh const* ms_boundMesh;
    };
}

#pragma once

#include "VertexLayout.h"

namespace vkr
{
    class Application;
    class Buffer;

    class Mesh
    {
    public:
        Mesh(Application const& app, Buffer const& buffer, VertexLayout layout);
        ~Mesh();

        VertexLayout const& getVertexLayout() const { return m_vertexLayout; }

        void bindBuffers(VkCommandBuffer commandBuffer) const;

        std::size_t getIndexCount() const { return m_vertexLayout.getIndexCount(); }

    public:
        static void resetBoundMesh() { ms_boundMesh = nullptr; }

    private:
        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        Buffer const& m_buffer;
        VertexLayout m_vertexLayout;

    private:
        static Mesh const* ms_boundMesh;
    };
}

#pragma once

#include "VertexLayout.h"

namespace vkr
{
    class Application;
    class Buffer;

    class Mesh
    {
    public:
        struct Metadata
        {
            bool hasColor = false;
            bool hasTexCoord = false;
            bool hasNormal = false;
            bool hasTangent = false;
        };

    public:
        Mesh(Application const& app, Buffer const& buffer, VertexLayout layout, Metadata metadata);
        ~Mesh();

        VertexLayout const& getVertexLayout() const { return m_vertexLayout; }
        Metadata const& getMetadata() const { return m_metadata; }

        void bindBuffers(VkCommandBuffer commandBuffer) const;

        std::size_t getIndexCount() const { return m_vertexLayout.getIndexCount(); }

    public:
        static void resetBoundMesh() { ms_boundMesh = nullptr; }

    private:
        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        Buffer const& m_buffer;
        VertexLayout m_vertexLayout;
        Metadata m_metadata;

    private:
        static Mesh const* ms_boundMesh; // TODO remove this hack
    };
}

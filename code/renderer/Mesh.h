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
        // TODO don't need Application
        Mesh(Application const& app, Buffer const& buffer, VertexLayout layout, Metadata metadata);
        ~Mesh();

        VertexLayout const& getVertexLayout() const { return m_vertexLayout; }
        Metadata const& getMetadata() const { return m_metadata; } // TODO not needed during rendering. Remove?

        void draw(VkCommandBuffer commandBuffer) const;

    private:
        void createBuffers(std::vector<unsigned char> const& rawData);

    private:
        Buffer const& m_buffer;
        VertexLayout m_vertexLayout;
        Metadata m_metadata;
    };
}

#pragma once

namespace vkgfx
{
    enum class BufferUsage
    {
        VertexIndexBuffer,
        UniformBuffer,
    };

    enum class BufferLocation
    {
        DeviceLocal,
        HostVisible,
    };

    struct BufferMetadata
    {
        BufferUsage usage = BufferUsage::VertexIndexBuffer;
        BufferLocation location = BufferLocation::DeviceLocal;
        bool isMutable = false;
    };
}

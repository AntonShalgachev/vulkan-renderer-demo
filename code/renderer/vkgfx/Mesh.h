#pragma once

#include "Handles.h"

namespace vkgfx
{
    enum class IndexType
    {
        UnsignedByte,
        UnsignedShort,
        UnsignedInt,
    };

    struct BufferWithOffset
    {
        BufferHandle buffer;
        std::size_t offset = 0;
    };

    // TODO store Vulkan objects directly
    struct Mesh
    {
        std::vector<BufferWithOffset> vertexBuffers;
        BufferWithOffset indexBuffer;
        std::size_t indexCount = 0;
        IndexType indexType = IndexType::UnsignedShort;
    };
}

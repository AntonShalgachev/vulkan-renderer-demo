#pragma once

#include "vkgfx/Handles.h"

#include "nstl/vector.h"

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
        size_t offset = 0;
    };

    // TODO store Vulkan objects directly
    struct Mesh
    {
        nstl::vector<BufferWithOffset> vertexBuffers;
        BufferWithOffset indexBuffer;
        size_t indexCount = 0;
        IndexType indexType = IndexType::UnsignedShort;
        size_t indexOffset = 0;
        size_t vertexOffset = 0;
    };
}

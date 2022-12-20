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
        std::size_t offset = 0;
    };

    // TODO store Vulkan objects directly
    struct Mesh
    {
        nstl::vector<BufferWithOffset> vertexBuffers;
        BufferWithOffset indexBuffer;
        std::size_t indexCount = 0;
        IndexType indexType = IndexType::UnsignedShort;
        std::size_t indexOffset = 0;
        std::size_t vertexOffset = 0;
    };
}
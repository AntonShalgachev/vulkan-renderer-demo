#pragma once

#include "Handles.h"

namespace vkgfx
{
    // TODO include primitives
    struct Mesh
    {
        BufferHandle vertexBuffer; // TODO possibly several buffers
        BufferHandle indexBuffer;
        std::size_t indexCount;
        // TOOD other mesh metadata
    };
}

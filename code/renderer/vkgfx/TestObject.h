#pragma once

#include "Handles.h"
#include <vector>
// #include <byte>

namespace vkgfx
{
    // TODO
    struct TestObject
    {
        vkgfx::MeshHandle mesh;
        vkgfx::MaterialHandle material;

        vkgfx::PipelineHandle pipeline;

        vkgfx::BufferHandle uniformBuffer;
        std::vector<std::byte> pushConstants; // TODO don't use dynamic memory
    };
}

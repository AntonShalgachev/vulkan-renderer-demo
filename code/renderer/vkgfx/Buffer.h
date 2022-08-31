#pragma once

#include "wrapper/DeviceMemory.h"
#include "wrapper/Buffer.h"

namespace vkgfx
{
    struct BufferHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct Buffer
    {
        Buffer(vko::DeviceMemory memory, vko::Buffer buffer);

        vko::DeviceMemory memory;
        vko::Buffer buffer;
    };
}

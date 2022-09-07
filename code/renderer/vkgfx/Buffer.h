#pragma once

#include "wrapper/DeviceMemory.h"
#include "wrapper/Buffer.h"
#include "vkgfx/BufferMetadata.h"

namespace vkgfx
{
    struct Buffer
    {
        Buffer(vko::DeviceMemory memory, vko::Buffer buffer, BufferMetadata metadata);

        vko::DeviceMemory memory;
        vko::Buffer buffer;
        BufferMetadata metadata;
    };
}

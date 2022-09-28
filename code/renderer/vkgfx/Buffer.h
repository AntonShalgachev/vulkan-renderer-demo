#pragma once

#include "wrapper/DeviceMemory.h"
#include "wrapper/Buffer.h"
#include "vkgfx/BufferMetadata.h"

namespace vkgfx
{
    struct Buffer
    {
        vko::DeviceMemory memory;
        vko::Buffer buffer;
        BufferMetadata metadata;

        std::size_t size = 0;
        std::size_t realSize = 0;

        // Only for mutable buffers
        std::vector<unsigned char> stagingBuffer;
        std::size_t stagingDirtyStart = 0;
        std::size_t stagingDirtyEnd = 0;
        std::size_t alignedSize = 0;

        std::size_t getDynamicOffset(std::size_t resourceIndex) const;
    };
}

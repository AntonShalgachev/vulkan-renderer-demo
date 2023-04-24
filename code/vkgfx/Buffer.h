#pragma once

#include "vko/DeviceMemory.h"
#include "vko/Buffer.h"
#include "vkgfx/BufferMetadata.h"

#include "nstl/vector.h"

namespace vkgfx
{
    struct Buffer
    {
        vko::DeviceMemory memory;
        vko::Buffer buffer;
        BufferMetadata metadata;

        size_t size = 0;
        size_t realSize = 0;

        // Only for mutable buffers
        nstl::vector<unsigned char> stagingBuffer;
        size_t stagingDirtyStart = 0;
        size_t stagingDirtyEnd = 0;
        size_t alignedSize = 0;

        size_t getDynamicOffset(size_t resourceIndex) const;
    };
}

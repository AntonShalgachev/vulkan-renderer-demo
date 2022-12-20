#pragma once

#include "renderer/wrapper/DeviceMemory.h"
#include "renderer/wrapper/Buffer.h"
#include "renderer/vkgfx/BufferMetadata.h"

#include "nstl/vector.h"

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
        nstl::vector<unsigned char> stagingBuffer;
        std::size_t stagingDirtyStart = 0;
        std::size_t stagingDirtyEnd = 0;
        std::size_t alignedSize = 0;

        std::size_t getDynamicOffset(std::size_t resourceIndex) const;
    };
}

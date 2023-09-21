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
        nstl::vector<vko::Buffer> buffers;
        BufferMetadata metadata;

        size_t size = 0;
        size_t alignedSize = 0;

        vko::Buffer const& getBuffer(size_t subresourceIndex) const;
    };
}

#include "Buffer.h"

std::size_t vkgfx::Buffer::getDynamicOffset(std::size_t resourceIndex) const
{
    if (!metadata.isMutable)
        return 0;

    return alignedSize * resourceIndex;
}

#include "Buffer.h"

size_t vkgfx::Buffer::getDynamicOffset(size_t resourceIndex) const
{
    if (!metadata.isMutable)
        return 0;

    return alignedSize * resourceIndex;
}

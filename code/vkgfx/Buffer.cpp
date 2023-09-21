#include "Buffer.h"

size_t vkgfx::Buffer::getSubresourceIndex(size_t subresourceCount) const
{
    return metadata.isMutable ? lastUpdatedFrameIndex % subresourceCount : 0;
}

vko::Buffer const& vkgfx::Buffer::getBuffer(size_t subresourceCount) const
{
    return buffers[getSubresourceIndex(subresourceCount)];
}

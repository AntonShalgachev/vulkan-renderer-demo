#include "Buffer.h"

vko::Buffer const& vkgfx::Buffer::getBuffer(size_t subresourceIndex) const
{
    size_t index = metadata.isMutable ? subresourceIndex : 0;
    return buffers[index];
}

#include "Buffer.h"

vkgfx::Buffer::Buffer(vko::DeviceMemory memory, vko::Buffer buffer, BufferMetadata metadata)
    : memory(std::move(memory))
    , buffer(std::move(buffer))
    , metadata(std::move(metadata))
{

}

std::size_t vkgfx::Buffer::getDynamicOffset(std::size_t resourceIndex) const
{
    if (!metadata.isMutable)
        return 0;

    return alignedSize * resourceIndex;
}

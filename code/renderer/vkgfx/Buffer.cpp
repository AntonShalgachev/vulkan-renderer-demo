#include "Buffer.h"

vkgfx::Buffer::Buffer(vko::DeviceMemory memory, vko::Buffer buffer, BufferMetadata metadata)
    : memory(std::move(memory))
    , buffer(std::move(buffer))
    , metadata(std::move(metadata))
{

}

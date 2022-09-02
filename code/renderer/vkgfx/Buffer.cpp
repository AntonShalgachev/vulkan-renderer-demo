#include "Buffer.h"

vkgfx::Buffer::Buffer(vko::DeviceMemory memory, vko::Buffer buffer, BufferUsage usage)
    : memory(std::move(memory))
    , buffer(std::move(buffer))
    , usage(usage)
{

}

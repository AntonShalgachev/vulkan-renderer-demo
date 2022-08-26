#include "Buffer.h"

vkgfx::Buffer::Buffer(vko::DeviceMemory memory, vko::Buffer buffer)
    : memory(std::move(memory))
    , buffer(std::move(buffer))
{

}

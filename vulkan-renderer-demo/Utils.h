#pragma once

#include "framework.h"

namespace vkr
{
    class Buffer;
    class DeviceMemory;

    namespace utils
    {
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory);
    }
}

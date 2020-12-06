#pragma once

#include "framework.h"

namespace vkr
{
    class Buffer;
    class DeviceMemory;
    class Image;

    namespace utils
    {
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory);
        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory);
    }
}

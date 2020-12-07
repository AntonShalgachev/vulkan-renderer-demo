#include "Utils.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "Image.h"

void vkr::utils::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory)
{
    buffer = std::make_unique<vkr::Buffer>(size, usage);
    bufferMemory = std::make_unique<vkr::DeviceMemory>(buffer->getMemoryRequirements(), properties);
    buffer->bind(*bufferMemory);
}

void vkr::utils::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory)
{
    image = std::make_unique<vkr::Image>(width, height, format, tiling, usage);
    imageMemory = std::make_unique<vkr::DeviceMemory>(image->getMemoryRequirements(), properties);
    image->bind(*imageMemory);
}
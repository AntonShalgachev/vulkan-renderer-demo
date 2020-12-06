#include "Utils.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include "ScopedOneTimeCommandBuffer.h"

void vkr::utils::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory)
{
    buffer = std::make_unique<vkr::Buffer>(size, usage);
    bufferMemory = std::make_unique<vkr::DeviceMemory>(buffer->getMemoryRequirements(), properties);
    buffer->bind(*bufferMemory);
}

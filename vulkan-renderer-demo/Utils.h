#pragma once

#include "framework.h"

namespace vkr
{
    class Buffer;
    class DeviceMemory;
    class Image;
    class Application;

    namespace utils
    {
        void createBuffer(vkr::Application const& app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Buffer>& buffer, std::unique_ptr<vkr::DeviceMemory>& bufferMemory);
        void createImage(vkr::Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory);

        // TODO extract somewhere else
        bool hasEveryOption(std::vector<char const*> const& availableOptions, std::vector<char const*> const& requestedOptions);
    }
}

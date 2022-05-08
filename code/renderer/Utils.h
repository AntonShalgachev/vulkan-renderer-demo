#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace vkr
{
    class DeviceMemory;
    class Image;
    class Application;

    namespace utils
    {
        void createImage(vkr::Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory);

        // TODO extract somewhere else
        bool hasEveryOption(std::vector<char const*> const& availableOptions, std::vector<char const*> const& requestedOptions);
    }
}

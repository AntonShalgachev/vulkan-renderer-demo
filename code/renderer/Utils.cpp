#include "Utils.h"
#include "wrapper/DeviceMemory.h"
#include "wrapper/Image.h"
#include <algorithm>

void vkr::utils::createImage(vkr::Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory)
{
    image = std::make_unique<vkr::Image>(app, width, height, format, tiling, usage);
    imageMemory = std::make_unique<vkr::DeviceMemory>(app, image->getMemoryRequirements(), properties);
    image->bindMemory(*imageMemory);
}

bool vkr::utils::hasEveryOption(std::vector<char const*> const& availableOptions, std::vector<char const*> const& requestedOptions)
{
    for (const auto& requestedOption : requestedOptions)
    {
        auto it = std::find_if(availableOptions.begin(), availableOptions.end(), [requestedOption](char const* availableOption)
        {
            return std::strcmp(availableOption, requestedOption) == 0;
        });

        if (it == availableOptions.end())
            return false;
    }

    return true;
}

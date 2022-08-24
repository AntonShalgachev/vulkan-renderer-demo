#include "Utils.h"

#include "wrapper/DeviceMemory.h"
#include "wrapper/Image.h"
#include "Application.h"

#include <algorithm>

void vkr::utils::createImage(vkr::Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vko::Image>& image, std::unique_ptr<vko::DeviceMemory>& imageMemory)
{
    image = std::make_unique<vko::Image>(app.getDevice(), width, height, format, tiling, usage);
    imageMemory = std::make_unique<vko::DeviceMemory>(app.getDevice(), app.getPhysicalDevice(), image->getMemoryRequirements(), properties);
    image->bindMemory(*imageMemory);
}

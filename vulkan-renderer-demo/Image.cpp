#include "Image.h"

#include "DeviceMemory.h"
#include "ImageView.h"

namespace vkr
{
    Image::Image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
    {
        m_format = format;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(temp::getDevice(), &imageCreateInfo, nullptr, &m_image) != VK_SUCCESS)
            throw std::runtime_error("failed to create image!");
    }

    Image::Image(VkImage image, VkFormat format)
    {
        m_isOwned = false;
        m_image = image;
        m_format = format;
    }

    Image::~Image()
    {
        if (m_isOwned)
            vkDestroyImage(temp::getDevice(), m_image, nullptr);
    }

    VkMemoryRequirements Image::getMemoryRequirements() const
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(temp::getDevice(), m_image, &memRequirements);

        return memRequirements;
    }

    void Image::bind(DeviceMemory const& memory) const
    {
        vkBindImageMemory(temp::getDevice(), m_image, memory.getHandle(), 0);
    }

    std::unique_ptr<vkr::ImageView> Image::createImageView(VkImageAspectFlags aspectFlags)
    {
        return std::make_unique<vkr::ImageView>(m_image, m_format, aspectFlags);
    }

}

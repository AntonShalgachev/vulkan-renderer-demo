#include "Image.h"

#include "Assert.h"
#include "DeviceMemory.h"
#include "Device.h"

#include <stdexcept>

namespace vko
{
    Image::Image(Device const& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) : m_device(device.getHandle())
    {
        m_isOwned = true;
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

        VKO_ASSERT(vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_handle.get()));
    }

    Image::Image(Device const& device, VkImage image, VkFormat format) : m_device(device.getHandle())
    {
        m_isOwned = false;
        m_handle = image;
        m_format = format;
    }

    Image::~Image()
    {
        if (m_isOwned)
            vkDestroyImage(m_device, m_handle, nullptr);
    }

    VkMemoryRequirements Image::getMemoryRequirements() const
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, m_handle, &memRequirements);

        return memRequirements;
    }

    void Image::bindMemory(DeviceMemory const& memory) const
    {
        VKO_ASSERT(vkBindImageMemory(m_device, m_handle, memory.getHandle(), 0));
    }
}

#pragma once

#include "framework.h"

namespace vkr
{
    class DeviceMemory;
    class ImageView;

    class Image
    {
    public:
        Image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
        ~Image();

        VkMemoryRequirements getMemoryRequirements() const;
        void bind(DeviceMemory const& memory) const;

        std::unique_ptr<ImageView> createImageView(VkImageAspectFlags aspectFlags);

        VkImage getHandle() const { return m_image; }
        VkFormat getFormat() const { return m_format; }

    private:
        VkImage m_image;
        VkFormat m_format;
    };
}

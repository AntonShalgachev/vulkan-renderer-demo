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
        Image(VkImage image, VkFormat format);
        ~Image();

        VkMemoryRequirements getMemoryRequirements() const;
        void bind(DeviceMemory const& memory) const;

        std::unique_ptr<ImageView> createImageView(VkImageAspectFlags aspectFlags);

        VkImage getHandle() const { return m_handle; }
        VkFormat getFormat() const { return m_format; }

    private:
        bool m_isOwned = true;
        VkImage m_handle;
        VkFormat m_format;
    };
}

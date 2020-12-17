#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class DeviceMemory;
    class ImageView;

    class Image : public Object
    {
    public:
        explicit Image(Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
        explicit Image(Application const& app, VkImage image, VkFormat format);
        ~Image();

        Image(Image const&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image const&) = delete;
        Image& operator=(Image&&) = delete;

        VkMemoryRequirements getMemoryRequirements() const;
        void bindMemory(DeviceMemory const& memory) const;

        std::unique_ptr<ImageView> createImageView(VkImageAspectFlags aspectFlags);

        VkImage getHandle() const { return m_handle; }
        VkFormat getFormat() const { return m_format; }

    private:
        bool m_isOwned = true;
        VkImage m_handle = VK_NULL_HANDLE;
        VkFormat m_format;
    };
}

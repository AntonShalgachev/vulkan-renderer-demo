#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include "UniqueHandle.h"

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

        Image(Image const&) = default;
        Image(Image&&) = default;
        Image& operator=(Image const&) = default;
        Image& operator=(Image&&) = default;

        VkMemoryRequirements getMemoryRequirements() const;
        void bindMemory(DeviceMemory const& memory) const;

        std::unique_ptr<ImageView> createImageView(VkImageAspectFlags aspectFlags);

        VkImage getHandle() const { return m_handle; }
        VkFormat getFormat() const { return m_format; }

    private:
        bool m_isOwned = true;
        UniqueHandle<VkImage> m_handle;
        VkFormat m_format;
    };
}

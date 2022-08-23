#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include "UniqueHandle.h"

namespace vkr
{
    class DeviceMemory;
    class ImageView;
    class Device;

    class Image
    {
    public:
        explicit Image(Device const& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
        explicit Image(Device const& device, VkImage image, VkFormat format);
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
        Device const& m_device;
        bool m_isOwned = true;
        UniqueHandle<VkImage> m_handle;
        VkFormat m_format;
    };
}

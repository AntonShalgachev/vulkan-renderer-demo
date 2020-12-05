#pragma once

#include "framework.h"

namespace vkr
{
    class DeviceMemory;

    class Image
    {
    public:
        Image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
        ~Image();

        VkMemoryRequirements getMemoryRequirements() const;
        void bind(DeviceMemory const& memory) const;

        VkImage getHandle() const { return m_image; }

    private:
        VkImage m_image;
    };
}

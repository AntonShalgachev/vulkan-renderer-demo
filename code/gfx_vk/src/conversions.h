#pragma once

#include <vulkan/vulkan.h>

namespace gfx
{
    enum class image_format;
    enum class image_type;
    enum class image_usage;
}

namespace gfx_vk::utils
{
    VkFormat get_format(gfx::image_format format);
    VkImageAspectFlags get_aspect_flags(gfx::image_type type);
    VkImageUsageFlags get_usage_flags(gfx::image_usage usage);
}

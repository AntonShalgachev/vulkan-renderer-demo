#pragma once

#include <vulkan/vulkan.h>

namespace gfx
{
    enum class image_format;
    enum class image_type;
    enum class image_usage;
    enum class descriptor_type;
    enum class index_type;
}

namespace gfx_vk
{
    enum class color_space;
}

namespace gfx_vk::utils
{
    VkFormat get_format(gfx::image_format format);
    VkColorSpaceKHR get_color_space(gfx_vk::color_space space);
    VkImageAspectFlags get_aspect_flags(gfx::image_type type);
    VkImageUsageFlags get_usage_flags(gfx::image_usage usage);
    VkDescriptorType get_descriptor_type(gfx::descriptor_type type);
    VkIndexType get_index_type(gfx::index_type type);
}

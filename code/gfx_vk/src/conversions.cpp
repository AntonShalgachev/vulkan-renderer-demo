#include "conversions.h"

#include "gfx/resources.h"

VkFormat gfx_vk::utils::get_format(gfx::image_format format)
{
    switch (format)
    {
    case gfx::image_format::r8g8b8a8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case gfx::image_format::b8g8r8a8_srgb:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case gfx::image_format::r8g8b8:
        return VK_FORMAT_R8G8B8_UNORM;
    case gfx::image_format::bc1_unorm:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case gfx::image_format::bc3_unorm:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case gfx::image_format::bc5_unorm:
        return VK_FORMAT_BC5_UNORM_BLOCK;
    case gfx::image_format::d32_float:
        return VK_FORMAT_D32_SFLOAT;
    }

    assert(false);
    return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags gfx_vk::utils::get_aspect_flags(gfx::image_type type)
{
    switch (type)
    {
    case gfx::image_type::color:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case gfx::image_type::depth:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    assert(false);
    return VK_IMAGE_ASPECT_NONE;
}

VkImageUsageFlags gfx_vk::utils::get_usage_flags(gfx::image_usage usage)
{
    switch (usage)
    {
    case gfx::image_usage::color:
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    case gfx::image_usage::depth_sampled:
        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    case gfx::image_usage::depth:
        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    case gfx::image_usage::upload_sampled:
        return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    assert(false);
    return 0;
}

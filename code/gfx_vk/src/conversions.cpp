#include "conversions.h"

#include "swapchain.h"

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

VkColorSpaceKHR gfx_vk::utils::get_color_space(gfx_vk::color_space space)
{
    switch (space)
    {
    case color_space::srgb: return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    assert(false);
    return VK_COLOR_SPACE_MAX_ENUM_KHR;
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

VkDescriptorType gfx_vk::utils::get_descriptor_type(gfx::descriptor_type type)
{
    switch (type)
    {
    case gfx::descriptor_type::uniform_buffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case gfx::descriptor_type::combined_image_sampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    assert(false);
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkIndexType gfx_vk::utils::get_index_type(gfx::index_type type)
{
    switch (type)
    {
    case gfx::index_type::uint16: return VK_INDEX_TYPE_UINT16;
    case gfx::index_type::uint32: return VK_INDEX_TYPE_UINT32;
    }

    assert(false);
    return VK_INDEX_TYPE_MAX_ENUM;
}

#include "conversions.h"

#include "gfx/resources.h"

VkFormat gfx_vk::utils::get_format(gfx::image_format format)
{
    switch (format)
    {
    case gfx::image_format::r8g8b8a8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case gfx::image_format::r8g8b8a8_srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
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
    return VK_FORMAT_R8G8B8A8_UNORM;
}

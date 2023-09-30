#pragma once

#include <vulkan/vulkan.h>

namespace gfx
{
    enum class image_format;
}

namespace gfx_vk::utils
{
    VkFormat get_format(gfx::image_format format);
}

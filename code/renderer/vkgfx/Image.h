#pragma once

#include "renderer/wrapper/DeviceMemory.h"
#include "renderer/wrapper/Image.h"
#include "renderer/wrapper/ImageView.h"

#include "renderer/vkgfx/ImageMetadata.h"

namespace vkgfx
{
    struct Image
    {
        vko::DeviceMemory memory;
        vko::Image image;
        vko::ImageView imageView;
        ImageMetadata metadata;
    };
}

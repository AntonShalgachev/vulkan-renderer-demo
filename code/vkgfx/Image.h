#pragma once

#include "vko/DeviceMemory.h"
#include "vko/Image.h"
#include "vko/ImageView.h"

#include "vkgfx/ImageMetadata.h"

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

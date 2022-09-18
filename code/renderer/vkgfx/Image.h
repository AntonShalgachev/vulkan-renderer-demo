#pragma once

#include "wrapper/DeviceMemory.h"
#include "wrapper/Image.h"
#include "wrapper/ImageView.h"

#include "ImageMetadata.h"

namespace vkgfx
{
    struct Image
    {
        Image(vko::DeviceMemory memory, vko::Image image, vko::ImageView imageView, ImageMetadata metadata);

        vko::DeviceMemory memory;
        vko::Image image;
        vko::ImageView imageView;
        ImageMetadata metadata;
        std::size_t byteSize = 0;
    };
}

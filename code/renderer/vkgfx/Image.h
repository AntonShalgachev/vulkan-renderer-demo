#pragma once

#include "wrapper/DeviceMemory.h"
#include "wrapper/Image.h"
#include "wrapper/ImageView.h"

namespace vkgfx
{
    enum class ImageFormat
    {
        R8G8B8A8,
        R8G8B8,
    };

    struct ImageHandle
    {
        std::size_t index; // TODO improve
    };

    struct ImageMetadata
    {
        std::size_t width;
        std::size_t height;
        ImageFormat format;
    };

    struct Image
    {
        Image(vko::DeviceMemory memory, vko::Image image, vko::ImageView imageView, ImageMetadata metadata);

        vko::DeviceMemory memory;
        vko::Image image;
        vko::ImageView imageView;
        ImageMetadata metadata;
    };
}

#pragma once

namespace vkgfx
{
    enum class ImageFormat
    {
        R8G8B8A8,
        R8G8B8,
        BC1_UNORM,
        BC3_UNORM,
        BC5_UNORM,
    };

    struct ImageMetadata
    {
        std::size_t width = 0;
        std::size_t height = 0;
        std::size_t bitsPerPixel = 0;
        ImageFormat format = ImageFormat::R8G8B8A8;
        bool srgb = false;
    };
}

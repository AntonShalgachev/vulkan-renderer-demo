#pragma once

namespace vkgfx
{
    enum class ImageFormat
    {
        R8G8B8A8,
        R8G8B8,
    };

    struct ImageMetadata
    {
        std::size_t width;
        std::size_t height;
        ImageFormat format;
    };
}

#pragma once

#include "framework.h"

namespace vkr
{
    class Image;
    class DeviceMemory;
    class ImageView;

    class Texture
    {
    public:
        explicit Texture(std::string const& path);

        VkImageView getImageViewHandle() const;

    private:
        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <string>

namespace vkr
{
    class Image;
    class DeviceMemory;
    class ImageView;

    class Texture : Object
    {
    public:
        explicit Texture(Application const& app, std::string const& path);

        VkImageView getImageViewHandle() const;

    private:
        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;
    };
}

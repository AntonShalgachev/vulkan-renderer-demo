#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <string>

namespace tinygltf
{
    struct Image;
}

namespace vkr
{
    class Image;
    class DeviceMemory;
    class ImageView;

    class Texture : Object
    {
    public:
        explicit Texture(Application const& app, std::string const& path);
        explicit Texture(Application const& app, tinygltf::Image const& image);

        VkImageView getImageViewHandle() const;

    private:
        void createImage(void const* data, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components);

        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;
    };
}

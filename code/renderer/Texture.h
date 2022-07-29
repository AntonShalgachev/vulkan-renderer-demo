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

    // TODO include Sampler here
    class Texture : Object
    {
    public:
        explicit Texture(Application const& app, tinygltf::Image const& image); // TODO remove this constructor
        ~Texture();

        VkImageView getImageViewHandle() const;

    private:
        void createImage(void const* data, std::size_t size, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components);

        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;
    };
}

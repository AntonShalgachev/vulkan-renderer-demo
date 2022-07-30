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
    class Sampler;

    class Texture : Object
    {
    public:
        explicit Texture(Application const& app, tinygltf::Image const& image, std::shared_ptr<vkr::Sampler> sampler); // TODO don't pass tinygltf::Image
        ~Texture();

        Sampler const& getSampler() const;
        ImageView const& getImageView() const;

    private:
        void createImage(void const* data, std::size_t size, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components);

        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;

        std::shared_ptr<vkr::Sampler> m_sampler;

        std::string m_name;
    };
}

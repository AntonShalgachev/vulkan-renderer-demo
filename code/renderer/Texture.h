#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <string>
#include <span>

namespace vkr
{
    class Image;
    class DeviceMemory;
    class ImageView;
    class Sampler;
    
    // TODO separate this object into 2: one which only contains image data, and the one which is a composition of the image data and the sampler
    class Texture : Object
    {
    public:
        // TODO use std::byte
        explicit Texture(Application const& app, std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components, std::shared_ptr<vkr::Sampler> sampler);
        ~Texture();

        void setName(std::string_view name);

        Sampler const& getSampler() const;
        ImageView const& getImageView() const;

    private:
        void createImage(std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components);

        std::unique_ptr<vkr::Image> m_image;
        std::unique_ptr<vkr::DeviceMemory> m_memory;
        std::unique_ptr<vkr::ImageView> m_imageView;

        std::shared_ptr<vkr::Sampler> m_sampler;

        std::string m_name;
    };
}

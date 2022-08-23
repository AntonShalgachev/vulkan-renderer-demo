#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <string>
#include <span>

namespace vko
{
    class Image;
    class DeviceMemory;
    class ImageView;
    class Sampler;
}

namespace vkr
{    
    // TODO separate this object into 2: one which only contains image data, and the one which is a composition of the image data and the sampler
    class Texture : Object
    {
    public:
        // TODO use std::byte
        explicit Texture(Application const& app, std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components, std::shared_ptr<vko::Sampler> sampler);
        ~Texture();

        void setName(std::string_view name);

        vko::Sampler const& getSampler() const;
        vko::ImageView const& getImageView() const;

    private:
        void createImage(std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components);

        std::unique_ptr<vko::Image> m_image;
        std::unique_ptr<vko::DeviceMemory> m_memory;
        std::unique_ptr<vko::ImageView> m_imageView;

        std::shared_ptr<vko::Sampler> m_sampler;

        std::string m_name;
    };
}

#include "Image.h"

vkgfx::Image::Image(vko::DeviceMemory memory, vko::Image image, vko::ImageView imageView, ImageMetadata metadata)
    : memory(std::move(memory))
    , image(std::move(image))
    , imageView(std::move(imageView))
    , metadata(std::move(metadata))
{

}

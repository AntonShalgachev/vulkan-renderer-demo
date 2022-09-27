#include "Texture.h"

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"
#include "Utils.h"
#include "wrapper/Image.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "wrapper/ImageView.h"
#include <stdexcept>
#include "BufferWithMemory.h"
#include "Application.h"

namespace
{
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }
}

vkr::Texture::~Texture()
{
    // TODO remove this hack (this ensures the objects are destroyed in the right order)
    m_imageView = nullptr;
    m_image = nullptr;
    m_memory = nullptr;
}

void vkr::Texture::setName(std::string_view name)
{
    m_name = std::string{ name };
}

vkr::Texture::Texture(Application const& app, std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components, std::shared_ptr<vko::Sampler> sampler) : Object(app), m_sampler(std::move(sampler))
{
    createImage(bytes, width, height, bitsPerComponent, components);
}

vko::Sampler const& vkr::Texture::getSampler() const
{
    return *m_sampler;
}

vko::ImageView const& vkr::Texture::getImageView() const
{
    return *m_imageView;
}

void vkr::Texture::createImage(std::span<unsigned char const> bytes, uint32_t width, uint32_t height, std::size_t bitsPerComponent, std::size_t components)
{
    std::size_t bytesPerPixel = components * bitsPerComponent / 8;

    std::size_t imageSize = width * height * bytesPerPixel;

    if (imageSize != bytes.size())
        throw std::runtime_error("Unexpected image size!");

    // TODO implement
    if (components != 4)
        throw std::runtime_error("Unexpected pixel format!");
    if (bitsPerComponent != 8)
        throw std::runtime_error("Unexpected pixel format!");
    if (bytesPerPixel != 4)
        throw std::runtime_error("Unexpected pixel format!");

    vkr::BufferWithMemory stagingBuffer{ getApp().getDevice(), getApp().getPhysicalDevice(), bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(bytes.data(), bytes.size());

    // TODO use SRGB for textures data and UNORM for normal maps
    vkr::utils::createImage(getApp(), width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);

    {
        vkr::ScopedOneTimeCommandBuffer commandBuffer{ getApp() };

        transitionImageLayout(commandBuffer.getHandle(), m_image->getHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // TODO extract to some class
        copyBufferToImage(commandBuffer.getHandle(), stagingBuffer.buffer().getHandle(), m_image->getHandle(), width, height);
        transitionImageLayout(commandBuffer.getHandle(), m_image->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        commandBuffer.submit();
    }

    m_imageView = std::make_unique<vko::ImageView>(m_image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT));
}

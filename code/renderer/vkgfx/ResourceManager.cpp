#include "ResourceManager.h"
#include "BufferWithMemory.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/CommandPool.h"
#include "wrapper/Queue.h"
#include "wrapper/ShaderModule.h"
#include "Image.h"
#include "Buffer.h"
#include "ShaderModule.h"
#include "Sampler.h"

namespace
{
    class OneTimeCommandBuffer
    {
    public:
        OneTimeCommandBuffer(vko::CommandPool const& commandPool, vko::Queue const& queue)
            : m_queue(queue)
            , m_buffers(commandPool.createCommandBuffers(1))
        {
            m_buffers.begin(0, true);
        }

        VkCommandBuffer getHandle()
        {
            return m_buffers.getHandle(0);
        }

        void submit()
        {
            m_buffers.end(0);

            m_buffers.submit(0, m_queue, nullptr, nullptr, nullptr);
            m_queue.waitIdle();
        }

    private:
        vko::Queue const& m_queue;

        vko::CommandBuffers m_buffers;
    };

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

vkgfx::ResourceManager::ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_uploadCommandPool(uploadCommandPool)
    , m_uploadQueue(uploadQueue)
{

}

vkgfx::ResourceManager::~ResourceManager() = default;

vkgfx::ImageHandle vkgfx::ResourceManager::createImage(ImageMetadata metadata)
{
    // TODO use SRGB for textures data and UNORM for normal maps
    VkFormat vkFormat = [](ImageFormat format) {
        switch (format)
        {
        case ImageFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        }

        throw std::invalid_argument("format");
    }(metadata.format);

    auto width = static_cast<uint32_t>(metadata.width);
    auto height = static_cast<uint32_t>(metadata.height);

    vko::Image vkImage{ m_device, width, height, vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
    vko::DeviceMemory vkImageMemory{ m_device, m_physicalDevice, vkImage.getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    vkImage.bindMemory(vkImageMemory);

    vko::ImageView vkImageView = vkImage.createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

    ImageHandle handle;
    handle.index = m_images.size();

    m_images.emplace_back(std::move(vkImageMemory), std::move(vkImage), std::move(vkImageView), std::move(metadata));

    return handle;
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, std::span<unsigned char const> bytes)
{
    Image const& image = m_images[handle.index];

    auto width = static_cast<uint32_t>(image.metadata.width);
    auto height = static_cast<uint32_t>(image.metadata.height);

    vkr::BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(bytes.data(), bytes.size());

    OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // TODO extract to some class
    copyBufferToImage(commandBuffer.getHandle(), stagingBuffer.buffer().getHandle(), image.image.getHandle(), width, height);
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer.submit();
}

vkgfx::BufferHandle vkgfx::ResourceManager::createBuffer(std::size_t size)
{
    vko::Buffer buffer{m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT }; // TODO configure externally
    vko::DeviceMemory memory{ m_device, m_physicalDevice, buffer.getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }; // TODO configure externally
    buffer.bindMemory(memory);

    BufferHandle handle;
    handle.index = m_buffers.size();

    m_buffers.emplace_back(std::move(memory), std::move(buffer));

    return handle;
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes)
{
    Buffer const& buffer = m_buffers[handle.index];

    vkr::BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(bytes.data(), bytes.size());

    OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
    vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), buffer.buffer);
    commandBuffer.submit();
}

vkgfx::ShaderModuleHandle vkgfx::ResourceManager::createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint)
{
    vko::ShaderModule module{ m_device, bytes, type, std::move(entryPoint) };

    ShaderModuleHandle handle;
    handle.index = m_shaderModules.size();

    m_shaderModules.emplace_back(std::move(module));

    return handle;
}

vkgfx::SamplerHandle vkgfx::ResourceManager::createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV)
{
    vko::Sampler sampler{ m_device, magFilter, minFilter, wrapU, wrapV };

    SamplerHandle handle;
    handle.index = m_samplers.size();

    m_samplers.emplace_back(std::move(sampler));

    return handle;
}

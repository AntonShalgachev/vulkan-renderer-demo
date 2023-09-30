#include "image.h"

#include "context.h"

#include "vko/Assert.h"
#include "vko/Buffer.h"
#include "vko/CommandBuffers.h"
#include "vko/CommandPool.h"
#include "vko/Device.h"
#include "vko/DeviceMemory.h"
#include "vko/Queue.h"

namespace
{
    VkFormat get_format(gfx::image_format format)
    {
        switch (format)
        {
        case gfx::image_format::r8g8b8a8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case gfx::image_format::r8g8b8:
            return VK_FORMAT_R8G8B8_UNORM;
        case gfx::image_format::bc1_unorm:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case gfx::image_format::bc3_unorm:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case gfx::image_format::bc5_unorm:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        }

        assert(false);
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

gfx_vk::image::image(context& context, gfx::image_params const& params)
    : m_context(context)
    , m_params(params)
{
    assert(params.width <= UINT32_MAX);
    assert(params.height <= UINT32_MAX);

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = static_cast<uint32_t>(params.width);
    imageCreateInfo.extent.height = static_cast<uint32_t>(params.height);
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = get_format(params.format);
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // TODO configure externally
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VKO_VERIFY(vkCreateImage(m_context.get_device().getHandle(), &imageCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(m_context.get_device().getHandle(), m_handle, &requirements);

    m_memory_size = requirements.size;

    m_memory = nstl::make_unique<vko::DeviceMemory>(m_context.get_device(), m_context.get_physical_device(), requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VKO_VERIFY(vkBindImageMemory(m_context.get_device().getHandle(), m_handle, m_memory->getHandle(), 0));
}

gfx_vk::image::~image()
{
    if (!m_handle)
        return;

    vkDestroyImage(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}

void gfx_vk::image::upload_sync(nstl::blob_view bytes)
{
    assert(bytes.size() <= m_memory_size);

    auto width = static_cast<uint32_t>(m_params.width);
    auto height = static_cast<uint32_t>(m_params.height);

    // TODO merge with buffer::upload_sync
    vko::Buffer staging_buffer{ m_context.get_device(), bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(m_context.get_device().getHandle(), staging_buffer.getHandle(), &requirements);
    vko::DeviceMemory staging_memory{ m_context.get_device(), m_context.get_physical_device(), requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    staging_buffer.bindMemory(staging_memory);
    staging_memory.copyFrom(bytes.data(), bytes.size());

    vko::CommandBuffers buffers = m_context.get_transfer_command_pool().allocate(1);
    buffers.begin(0, true);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_handle.get();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        buffers.getHandle(0),
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },

        .imageOffset = { 0, 0, 0 },
        .imageExtent = {
            width,
            height,
            1
        },
    };

    vkCmdCopyBufferToImage(
        buffers.getHandle(0),
        staging_buffer.getHandle(),
        m_handle.get(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(
        buffers.getHandle(0),
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    buffers.end(0);
    buffers.submit(0, m_context.get_transfer_queue(), nullptr, nullptr, nullptr);
    m_context.get_transfer_queue().waitIdle();
}

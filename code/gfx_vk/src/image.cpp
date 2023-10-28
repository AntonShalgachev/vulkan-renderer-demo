#include "image.h"

#include "context.h"
#include "conversions.h"

gfx_vk::image::image(context& context, gfx::image_params const& params, VkImage handle)
    : m_context(context)
    , m_params(params)
{
    assert(params.width <= UINT32_MAX);
    assert(params.height <= UINT32_MAX);

    VkFormat format = utils::get_format(params.format);

    m_owns_image = handle == VK_NULL_HANDLE;

    if (m_owns_image)
    {
        VkImageCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = { static_cast<uint32_t>(params.width), static_cast<uint32_t>(params.height), 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = utils::get_usage_flags(params.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        GFX_VK_VERIFY(vkCreateImage(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(m_context.get_device_handle(), m_handle, &requirements);

        m_memory_size = requirements.size;

        m_allocation = m_context.get_memory().allocate(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        allocation_data const* data = m_context.get_memory().get_data(m_allocation);
        assert(data);

        GFX_VK_VERIFY(vkBindImageMemory(m_context.get_device_handle(), m_handle, data->handle, data->offset));
    }
    else
    {
        m_handle = handle;
    }

    // TODO: image view shouldn't probably be created here. Revisit later
    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = utils::get_aspect_flags(params.type),
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    GFX_VK_VERIFY(vkCreateImageView(m_context.get_device_handle(), &view_info, &m_context.get_allocator(), &m_view_handle.get()));
}

gfx_vk::image::~image()
{
    if (m_owns_image && m_handle)
        vkDestroyImage(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
    m_handle = nullptr;

    if (m_view_handle)
        vkDestroyImageView(m_context.get_device_handle(), m_view_handle, &m_context.get_allocator());
    m_view_handle = nullptr;

    m_context.get_memory().free(m_allocation);
    m_allocation = {};
}

void gfx_vk::image::upload_sync(gfx::data_reader& reader)
{
    assert(reader.get_size() <= m_memory_size);

    auto width = static_cast<uint32_t>(m_params.width);
    auto height = static_cast<uint32_t>(m_params.height);

    transfer_data data = m_context.get_transfers().begin_transfer(reader);

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_handle.get(),
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(
        data.command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    assert(data.buffer_offset == 0); // TODO test non-zero offsets
    VkBufferImageCopy region{
        .bufferOffset = data.buffer_offset,
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
        data.command_buffer,
        data.buffer,
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
        data.command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    m_context.get_transfers().submit_and_wait(data.index);
}

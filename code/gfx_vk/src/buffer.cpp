#include "buffer.h"

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
    VkBufferUsageFlags get_usage(gfx::buffer_usage usage)
    {
        VkBufferUsageFlags persistent_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT; // TODO remove

        switch (usage)
        {
        case gfx::buffer_usage::vertex_index:
            return persistent_flags | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case gfx::buffer_usage::uniform:
            return persistent_flags | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        assert(false);
        return 0;
    }

    VkMemoryPropertyFlags get_memory_flags(gfx::buffer_location location)
    {
        switch (location)
        {
        case gfx::buffer_location::device_local:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case gfx::buffer_location::host_visible:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        assert(false);
        return 0;
    }
}

gfx_vk::buffer::buffer(context& context, gfx::buffer_params const& params)
    : m_context(context)
    , m_params(params)
{
    assert(!params.is_mutable); // TODO implement

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = params.size;
    bufferCreateInfo.usage = get_usage(params.usage);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VKO_VERIFY(vkCreateBuffer(context.get_device().getHandle(), &bufferCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(m_context.get_device().getHandle(), m_handle, &requirements);

    m_memory = nstl::make_unique<vko::DeviceMemory>(m_context.get_device(), m_context.get_physical_device(), requirements, get_memory_flags(params.location));

    size_t memory_offset = 0; // TODO use offset after implementing mutable buffers
    VKO_VERIFY(vkBindBufferMemory(m_context.get_device().getHandle(), m_handle, m_memory->getHandle(), memory_offset));
}

gfx_vk::buffer::~buffer()
{
    if (!m_handle)
        return;

    vkDestroyBuffer(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
}

void gfx_vk::buffer::upload_sync(nstl::blob_view bytes, size_t offset)
{
    if (m_params.location == gfx::buffer_location::device_local)
    {
        vko::Buffer staging_buffer{ m_context.get_device(), bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(m_context.get_device().getHandle(), staging_buffer.getHandle(), &requirements);
        vko::DeviceMemory staging_memory{ m_context.get_device(), m_context.get_physical_device(), requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        staging_buffer.bindMemory(staging_memory);
        staging_memory.copyFrom(bytes.data(), bytes.size());

        vko::CommandBuffers buffers = m_context.get_transfer_command_pool().allocate(1);
        buffers.begin(0, true);

        size_t memory_offset = 0; // TODO use offset after implementing mutable buffers

        VkBufferCopy copyRegion{};
        copyRegion.dstOffset = offset + memory_offset;
        copyRegion.srcOffset = 0;
        copyRegion.size = bytes.size();
        vkCmdCopyBuffer(buffers.getHandle(0), staging_buffer.getHandle(), m_handle, 1, &copyRegion);

        buffers.end(0);
        buffers.submit(0, m_context.get_transfer_queue(), nullptr, nullptr, nullptr);
        m_context.get_transfer_queue().waitIdle();
    }
    else
    {
        size_t memory_offset = 0; // TODO use offset after implementing mutable buffers
        m_memory->copyFrom(bytes.data(), bytes.size(), memory_offset);
    }
}

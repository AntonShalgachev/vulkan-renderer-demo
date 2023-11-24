#include "buffer.h"

#include "context.h"
#include "utils.h"

#include "nstl/alignment.h"

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
        case gfx::buffer_usage::storage:
            return persistent_flags | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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

struct gfx_vk::buffer::impl
{
    impl(context& context, VkBufferCreateInfo const& info) : context(context)
    {
        GFX_VK_VERIFY(vkCreateBuffer(context.get_device_handle(), &info, &context.get_allocator(), &handle.get()));
    }

    impl(impl&& rhs) = default;

    ~impl()
    {
        if (!handle)
            return;

        vkDestroyBuffer(context.get_device_handle(), handle, &context.get_allocator());
        handle = nullptr;
    }

    context& context;
    unique_handle<VkBuffer> handle;
};

gfx_vk::buffer::buffer(context& context, gfx::buffer_params const& params)
    : m_context(context)
    , m_params(params)
{
    VkBufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = params.size,
        .usage = get_usage(params.usage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    size_t resource_count = params.is_mutable ? context.get_mutable_resource_multiplier() : 1;

    for (size_t i = 0; i < resource_count; i++)
        m_buffers.emplace_back(m_context, info);

    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(m_context.get_device_handle(), m_buffers[0].handle, &requirements);

    m_aligned_size = nstl::align_up(requirements.size, requirements.alignment);

    requirements.size = m_aligned_size * resource_count;

    m_allocation = m_context.get_memory().allocate(requirements, get_memory_flags(params.location));
    allocation_data const* data = m_context.get_memory().get_data(m_allocation);
    assert(data);

    for (size_t i = 0; i < resource_count; i++)
    {
        size_t memory_offset = i * m_aligned_size;
        GFX_VK_VERIFY(vkBindBufferMemory(m_context.get_device_handle(), m_buffers[i].handle, data->handle, data->offset + memory_offset));
    }
}

gfx_vk::buffer::~buffer()
{
    m_context.get_memory().free(m_allocation);
    m_allocation = {};
}

VkBuffer gfx_vk::buffer::get_handle(size_t index) const
{
    return m_buffers[index].handle;
}

VkBuffer gfx_vk::buffer::get_current_handle() const
{
    size_t index = m_params.is_mutable ? m_context.get_mutable_resource_index() : 0;
    return get_handle(index);
}

void gfx_vk::buffer::upload_sync(gfx::data_reader& reader, size_t offset)
{
    // TODO prevent calling this function on immutable resource

    size_t subresource_index = m_params.is_mutable ? m_context.get_mutable_resource_index() : 0;

    if (m_params.location == gfx::buffer_location::device_local)
    {
        transfer_data data = m_context.get_transfers().begin_transfer(reader);

        VkBufferCopy region{
            .srcOffset = data.buffer_offset,
            .dstOffset = offset,
            .size = reader.get_size(),
        };
        vkCmdCopyBuffer(data.command_buffer, data.buffer, m_buffers[subresource_index].handle, 1, &region);

        m_context.get_transfers().submit_and_wait(data.index);
    }
    else
    {
        size_t memory_offset = subresource_index * m_aligned_size + offset;

        allocation_data const* data = m_context.get_memory().get_data(m_allocation);
        assert(data);
        assert(data->ptr);

        if (!reader.read(static_cast<unsigned char*>(data->ptr) + memory_offset, reader.get_size()))
            assert(false);
//         memcpy(static_cast<unsigned char*>(data->ptr) + memory_offset, bytes.data(), bytes.size());
    }
}

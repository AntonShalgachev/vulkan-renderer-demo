#include "buffer.h"

#include "context.h"
#include "utils.h"

#include "vko/Buffer.h"
#include "vko/CommandBuffers.h"
#include "vko/CommandPool.h"
#include "vko/DeviceMemory.h"

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

    // TODO move somewhere
    size_t align_up(size_t value, size_t alignment)
    {
        // TODO assert that alignment is a power of 2

        if (alignment == 0)
            return value;

        return (value + alignment - 1) & ~(alignment - 1);
    }
}

struct gfx_vk::buffer::impl
{
    impl(context& context, VkBufferCreateInfo const& info) : context(context)
    {
        GFX_VK_VERIFY(vkCreateBuffer(context.get_device_handle(), &info, &context.get_allocator(), &handle.get()));
    }

    impl(impl const&) = default;
    impl(impl&& rhs) = default;

    ~impl()
    {
        if (!handle)
            return;

        vkDestroyBuffer(context.get_device_handle(), handle, &context.get_allocator());
        handle = nullptr;
    }

    impl& operator=(impl const&) = default;
    impl& operator=(impl&& rhs) = default;

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

    m_aligned_size = align_up(requirements.size, requirements.alignment);

    requirements.size = m_aligned_size * resource_count;

    m_memory = nstl::make_unique<vko::DeviceMemory>(m_context.get_device_handle(), m_context.get_physical_device_handle(), requirements, get_memory_flags(params.location));

    for (size_t i = 0; i < resource_count; i++)
    {
        size_t memory_offset = i * m_aligned_size;
        GFX_VK_VERIFY(vkBindBufferMemory(m_context.get_device_handle(), m_buffers[i].handle, m_memory->getHandle(), memory_offset));
    }
}

gfx_vk::buffer::~buffer() = default;

VkBuffer gfx_vk::buffer::get_handle(size_t index) const
{
    return m_buffers[index].handle;
}

VkBuffer gfx_vk::buffer::get_current_handle() const
{
    size_t index = m_params.is_mutable ? m_context.get_mutable_resource_index() : 0;
    return get_handle(index);
}

void gfx_vk::buffer::upload_sync(nstl::blob_view bytes, size_t offset)
{
    // TODO prevent calling this function on immutable resource

    size_t subresource_index = m_params.is_mutable ? m_context.get_mutable_resource_index() : 0;

    if (m_params.location == gfx::buffer_location::device_local)
    {
        // TODO merge with image::upload_sync
        vko::Buffer staging_buffer{ m_context.get_device_handle(), bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(m_context.get_device_handle(), staging_buffer.getHandle(), &requirements);
        vko::DeviceMemory staging_memory{ m_context.get_device_handle(), m_context.get_physical_device_handle(), requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        staging_buffer.bindMemory(staging_memory);
        staging_memory.copyFrom(bytes.data(), bytes.size());

        vko::CommandBuffers buffers = m_context.get_transfer_command_pool().allocate(1);
        buffers.begin(0, true);

        VkBufferCopy region{
            .srcOffset = 0,
            .dstOffset = offset,
            .size = bytes.size(),
        };
        vkCmdCopyBuffer(buffers.getHandle(0), staging_buffer.getHandle(), m_buffers[subresource_index].handle, 1, &region);

        buffers.end(0);
        buffers.submit(0, m_context.get_transfer_queue_handle(), nullptr, nullptr, nullptr);
        m_context.wait_idle(); // TODO remove
    }
    else
    {
        size_t memory_offset = subresource_index * m_aligned_size + offset;
        m_memory->copyFrom(bytes.data(), bytes.size(), memory_offset);
    }
}

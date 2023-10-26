#include "transfers.h"

#include "context.h"
#include "memory.h"

namespace
{
    struct buffer
    {
        buffer(gfx_vk::context& context, size_t size) : context(context)
        {
            VkBufferCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = size,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            GFX_VK_VERIFY(vkCreateBuffer(context.get_device_handle(), &info, &context.get_allocator(), &handle.get()));
        }

        buffer(buffer&& rhs) = default;

        ~buffer()
        {
            if (!handle)
                return;

            vkDestroyBuffer(context.get_device_handle(), handle, &context.get_allocator());
            handle = nullptr;
        }

        gfx_vk::context& context;
        gfx_vk::unique_handle<VkBuffer> handle;
    };
}

struct gfx_vk::transfers::transfer_storage
{
    ::buffer staging_buffer;
    allocation_handle allocation;
};

gfx_vk::transfers::transfers(context& context)
    : m_context(context)
    , m_command_pool(m_context, m_context.get_transfer_queue_family_index())
{
    m_command_buffer = m_command_pool.allocate();
}

gfx_vk::transfers::~transfers()
{
    for (allocation_handle handle : m_allocations)
        m_context.get_memory().free(handle);
}

gfx_vk::transfer_data gfx_vk::transfers::begin_transfer(nstl::blob_view bytes)
{
    // TODO have preallocated memory and buffers

    size_t index = m_transfers.size();

    ::buffer staging_buffer{m_context, bytes.size()};

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(m_context.get_device_handle(), staging_buffer.handle, &requirements);

    allocation_handle allocation = m_context.get_memory().allocate(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    allocation_data const* data = m_context.get_memory().get_data(allocation);
    assert(data);
    assert(data->ptr);
    memcpy(data->ptr, bytes.data(), bytes.size());

    GFX_VK_VERIFY(vkBindBufferMemory(m_context.get_device_handle(), staging_buffer.handle, data->handle, data->offset));

    VkCommandPoolResetFlags flags = 0;
    GFX_VK_VERIFY(vkResetCommandPool(m_context.get_device_handle(), m_command_pool.get_handle(), flags));

    VkCommandBufferBeginInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    GFX_VK_VERIFY(vkBeginCommandBuffer(m_command_buffer, &info));

    VkBuffer staging_buffer_handle = staging_buffer.handle;

    m_transfers.push_back({
        .staging_buffer = nstl::move(staging_buffer),
        .allocation = allocation,
    });

    return { index, staging_buffer_handle, 0, m_command_buffer };
}

void gfx_vk::transfers::submit_and_wait(size_t index)
{
    assert(m_transfers.size() == 1);
    assert(index == 0);

    GFX_VK_VERIFY(vkEndCommandBuffer(m_command_buffer));

    VkSubmitInfo info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
    };

    GFX_VK_VERIFY(vkQueueSubmit(m_context.get_transfer_queue_handle(), 1, &info, VK_NULL_HANDLE));

    vkQueueWaitIdle(m_context.get_transfer_queue_handle()); // TODO remove

    m_context.get_memory().free(m_transfers[index].allocation);

    m_transfers.pop_back();
}

#include "transfers.h"

#include "context.h"

namespace
{
    static constexpr size_t STAGING_BUFFER_SIZE = 64 * 1024 * 1024;
}

struct gfx_vk::transfers::transfer_storage
{

};

gfx_vk::transfers::transfers(context& context)
    : m_context(context)
    , m_command_pool(m_context, m_context.get_instance().get_transfer_queue_family_index())
{
    VkBufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = STAGING_BUFFER_SIZE,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    GFX_VK_VERIFY(vkCreateBuffer(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_buffer.get()));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(m_context.get_device_handle(), m_buffer, &requirements);

    m_memory = m_context.get_memory().allocate(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    allocation_data const* data = m_context.get_memory().get_data(m_memory);
    assert(data);
    GFX_VK_VERIFY(vkBindBufferMemory(m_context.get_device_handle(), m_buffer, data->handle, data->offset));

    m_command_buffer = m_command_pool.allocate();
}

gfx_vk::transfers::~transfers()
{
    if (m_buffer)
    {
        vkDestroyBuffer(m_context.get_device_handle(), m_buffer, &m_context.get_allocator());
        m_buffer = nullptr;
    }

    m_context.get_memory().free(m_memory);
}

gfx_vk::transfer_data gfx_vk::transfers::begin_transfer(gfx::data_reader& reader)
{
    size_t index = m_transfers.size();

    assert(reader.get_size() <= STAGING_BUFFER_SIZE);

    allocation_data const* data = m_context.get_memory().get_data(m_memory);
    assert(data);
    assert(data->ptr);

    if (!reader.read(data->ptr, reader.get_size()))
        assert(false);

    VkCommandPoolResetFlags flags = 0;
    GFX_VK_VERIFY(vkResetCommandPool(m_context.get_device_handle(), m_command_pool.get_handle(), flags));

    VkCommandBufferBeginInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    GFX_VK_VERIFY(vkBeginCommandBuffer(m_command_buffer, &info));

    m_transfers.push_back({

    });

    return { index, m_buffer, 0, m_command_buffer };
}

void gfx_vk::transfers::submit_and_wait([[maybe_unused]] size_t index)
{
    assert(m_transfers.size() == 1);
    assert(index == 0);

    GFX_VK_VERIFY(vkEndCommandBuffer(m_command_buffer));

    VkSubmitInfo info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
    };

    GFX_VK_VERIFY(vkQueueSubmit(m_context.get_instance().get_transfer_queue_handle(), 1, &info, VK_NULL_HANDLE));

    m_context.get_instance().wait_idle();

    m_transfers.pop_back();
}

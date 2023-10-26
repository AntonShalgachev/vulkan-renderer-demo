#include "command_pool.h"

#include "context.h"

gfx_vk::command_pool::command_pool(context& context, uint32_t queue_family) : m_context(context)
{
    VkCommandPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queue_family,
    };

    GFX_VK_VERIFY(vkCreateCommandPool(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));
}

gfx_vk::command_pool::~command_pool()
{
    vkDestroyCommandPool(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
    m_handle = nullptr;
}

VkCommandBuffer gfx_vk::command_pool::allocate()
{
    VkCommandBufferAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer result = VK_NULL_HANDLE;
    GFX_VK_VERIFY(vkAllocateCommandBuffers(m_context.get_device_handle(), &info, &result));
    return result;
}

#include "CommandPool.h"

#include "vko/Assert.h"
#include "vko/Device.h"
#include "vko/QueueFamily.h"
#include "vko/CommandBuffers.h"

vko::CommandPool::CommandPool(VkDevice device, uint32_t family_index)
    : m_device(device)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = family_index;
    poolCreateInfo.flags = 0; // TODO make use of it

    VKO_VERIFY(vkCreateCommandPool(m_device, &poolCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

void vko::CommandPool::reset() const
{
    VkCommandPoolResetFlags flags = 0;
    VKO_VERIFY(vkResetCommandPool(m_device, m_handle, flags));
}

vko::CommandPool::~CommandPool()
{
    if (m_handle)
        vkDestroyCommandPool(m_device, m_handle, &m_allocator.getCallbacks());
}

vko::CommandBuffers vko::CommandPool::allocate(size_t size) const
{
    return CommandBuffers{ m_device, getHandle(), size };
}

#include "CommandPool.h"

#include "vko/Assert.h"
#include "vko/Device.h"
#include "vko/QueueFamily.h"
#include "vko/CommandBuffers.h"

vko::CommandPool::CommandPool(Device const& device, QueueFamily const& queueFamily)
    : m_device(device)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queueFamily.getIndex();
    poolCreateInfo.flags = 0; // TODO make use of it

    VKO_VERIFY(vkCreateCommandPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle.get()));
}

void vko::CommandPool::reset() const
{
    VkCommandPoolResetFlags flags = 0;
    VKO_VERIFY(vkResetCommandPool(m_device.getHandle(), m_handle, flags));
}

vko::CommandPool::~CommandPool()
{
    vkDestroyCommandPool(m_device.getHandle(), m_handle, nullptr);
}

vko::CommandBuffers vko::CommandPool::allocate(size_t size) const
{
    return CommandBuffers{ m_device, *this, size };
}

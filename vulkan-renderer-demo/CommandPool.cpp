#include "CommandPool.h"
#include "Device.h"
#include "QueueFamily.h"

vkr::CommandPool::CommandPool(Device const& device, QueueFamily const& queueFamily) : m_device(device)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queueFamily.getIndex();
    poolCreateInfo.flags = 0;

    if (vkCreateCommandPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

vkr::CommandPool::~CommandPool()
{
    vkDestroyCommandPool(m_device.getHandle(), m_handle, nullptr);
}

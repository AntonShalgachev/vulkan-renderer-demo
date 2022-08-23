#include "CommandPool.h"
#include "Device.h"
#include "QueueFamily.h"
#include "CommandBuffers.h"
#include "CommandBuffer.h"
#include <stdexcept>

vko::CommandPool::CommandPool(Device const& device, QueueFamily const& queueFamily)
    : m_device(device)
    , m_queueFamily(queueFamily)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = m_queueFamily.getIndex();
    poolCreateInfo.flags = 0; // TODO make use of it

    if (vkCreateCommandPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void vko::CommandPool::reset() const
{
    VkCommandPoolResetFlags flags = 0;
    VKR_ASSERT(vkResetCommandPool(m_device.getHandle(), m_handle, flags));
}

vko::CommandPool::~CommandPool()
{
    vkDestroyCommandPool(m_device.getHandle(), m_handle, nullptr);
}

vkr::CommandBuffer vko::CommandPool::createCommandBuffer() const
{
    return createCommandBuffers(1)[0];
}

std::vector<vkr::CommandBuffer> vko::CommandPool::createCommandBuffers(std::size_t size) const
{
    std::shared_ptr<CommandBuffers> container = std::make_shared<CommandBuffers>(m_device, *this, size);

    std::vector<vkr::CommandBuffer> commandBuffers;
    commandBuffers.reserve(size);
    for (std::size_t i = 0; i < size; i++)
        commandBuffers.emplace_back(container, i);

    return commandBuffers;
}

#include "CommandPool.h"
#include "Device.h"
#include "QueueFamily.h"
#include "Application.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "CommandBuffers.h"
#include "CommandBuffer.h"
#include <stdexcept>

vkr::CommandPool::CommandPool(Application const& app)
    : Object(app)
    , m_queueFamily(app.getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily())
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = m_queueFamily.getIndex();
    poolCreateInfo.flags = 0; // TODO make use of it

    if (vkCreateCommandPool(getApp().getDevice().getHandle(), &poolCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void vkr::CommandPool::reset() const
{
    VkCommandPoolResetFlags flags = 0;
    vkResetCommandPool(getDevice().getHandle(), m_handle, flags);
}

vkr::CommandPool::~CommandPool()
{
    vkDestroyCommandPool(getApp().getDevice().getHandle(), m_handle, nullptr);
}

vkr::CommandBuffer vkr::CommandPool::createCommandBuffer() const
{
    return createCommandBuffers(1)[0];
}

std::vector<vkr::CommandBuffer> vkr::CommandPool::createCommandBuffers(std::size_t size) const
{
    std::shared_ptr<CommandBuffers> container = std::make_shared<CommandBuffers>(getApp(), *this, size);

    std::vector<vkr::CommandBuffer> commandBuffers;
    commandBuffers.reserve(size);
    for (std::size_t i = 0; i < size; i++)
        commandBuffers.emplace_back(container, i);

    return commandBuffers;
}

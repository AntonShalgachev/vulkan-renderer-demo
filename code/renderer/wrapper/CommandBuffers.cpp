#include "CommandBuffers.h"

#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"
#include "CommandPool.h"
#include "QueueFamily.h"

vkr::CommandBuffers::CommandBuffers(Device const& device, CommandPool const& commandPool, std::size_t size)
    : m_device(device)
    , m_commandPool(commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = m_commandPool.getHandle();
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(size);

    m_handles.resize(size);
    VKR_ASSERT(vkAllocateCommandBuffers(m_device.getHandle(), &commandBufferAllocateInfo, m_handles.data()));
}

vkr::CommandBuffers::~CommandBuffers()
{
    vkFreeCommandBuffers(m_device.getHandle(), m_commandPool.getHandle(), static_cast<uint32_t>(m_handles.size()), m_handles.data());
}

#include "CommandBuffers.h"

#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"
#include "CommandPool.h"
#include "QueueFamily.h"

vkr::CommandBuffers::CommandBuffers(Application const& app, CommandPool const& commandPool, std::size_t size)
    : Object(app)
    , m_commandPool(commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = m_commandPool.getHandle();
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(size);

    m_handles.resize(size);
    vkAllocateCommandBuffers(getDevice().getHandle(), &commandBufferAllocateInfo, m_handles.data());
}

vkr::CommandBuffers::~CommandBuffers()
{
    vkFreeCommandBuffers(getDevice().getHandle(), m_commandPool.getHandle(), static_cast<uint32_t>(m_handles.size()), m_handles.data());
}

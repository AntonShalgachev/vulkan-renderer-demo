#include "CommandBuffers.h"

#include "Renderer.h"
#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"
#include "Queue.h"
#include "CommandPool.h"
#include "QueueFamily.h"

vkr::CommandBuffers::CommandBuffers(Application const& app, CommandPool const& commandPool, Queue const& queue, std::size_t size)
    : Object(app)
    , m_commandPool(commandPool)
    , m_queue(queue)
{
    if (commandPool.getQueueFamily().getIndex() != queue.getFamily().getIndex())
        throw std::runtime_error("Queue doesn't match queue family of the command pool");

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

void vkr::CommandBuffers::begin(std::size_t index, VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_handles[index], &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
}

void vkr::CommandBuffers::end(std::size_t index)
{
    if (vkEndCommandBuffer(m_handles[index]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void vkr::CommandBuffers::submitAndWait(std::size_t index)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_handles[index];

    vkQueueSubmit(m_queue.getHandle(), 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(m_queue.getHandle());
}

void vkr::CommandBuffers::submit(std::size_t index, Semaphore const& signalSemaphore, Semaphore const& waitSemaphore, Fence const& signalFence)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_handles[index];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore.getHandle();
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore.getHandle();
    submitInfo.pWaitDstStageMask = waitStages;

    if (vkQueueSubmit(m_queue.getHandle(), 1, &submitInfo, signalFence.getHandle()) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");
}

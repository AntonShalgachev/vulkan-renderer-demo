#include "CommandBuffer.h"
#include "CommandBuffers.h"
#include "Semaphore.h"
#include "Fence.h"
#include "Queue.h"

vkr::CommandBuffer::CommandBuffer(std::shared_ptr<CommandBuffers> const& container, std::size_t index)
    : m_container(container)
    , m_index(index)
{
    
}

VkCommandBuffer const& vkr::CommandBuffer::getHandle() const
{
    return m_container->getHandle(m_index);
}

void vkr::CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(getHandle(), &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
}

void vkr::CommandBuffer::end()
{
    if (vkEndCommandBuffer(getHandle()) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void vkr::CommandBuffer::submitAndWait(Queue const& queue)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &getHandle();

    vkQueueSubmit(queue.getHandle(), 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(queue.getHandle());
}

void vkr::CommandBuffer::submit(Queue const& queue, Semaphore const& signalSemaphore, Semaphore const& waitSemaphore, Fence const& signalFence)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &getHandle();

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore.getHandle();
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore.getHandle();
    submitInfo.pWaitDstStageMask = waitStages;

    if (vkQueueSubmit(queue.getHandle(), 1, &submitInfo, signalFence.getHandle()) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");
}
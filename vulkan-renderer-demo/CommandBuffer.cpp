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

void vkr::CommandBuffer::reset() const
{
    vkResetCommandBuffer(getHandle(), 0);
}

void vkr::CommandBuffer::begin(bool oneTime) const
{
    VkCommandBufferUsageFlags flags = 0;
    if (oneTime)
        flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(getHandle(), &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
}

void vkr::CommandBuffer::end() const
{
    if (vkEndCommandBuffer(getHandle()) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void vkr::CommandBuffer::submit(Queue const& queue, Semaphore const* signalSemaphore, Semaphore const* waitSemaphore, Fence const* signalFence) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &getHandle();

    if (signalSemaphore)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore->getHandle();
    }
    
    if (waitSemaphore)
    {
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore->getHandle();
        submitInfo.pWaitDstStageMask = waitStages;
    }

    VkFence signalFenceHandle = signalFence ? signalFence->getHandle() : VK_NULL_HANDLE;

    if (vkQueueSubmit(queue.getHandle(), 1, &submitInfo, signalFenceHandle) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");
}

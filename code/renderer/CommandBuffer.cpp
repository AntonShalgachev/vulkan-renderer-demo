#include "CommandBuffer.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/Semaphore.h"
#include "wrapper/Fence.h"
#include "wrapper/Queue.h"
#include <array>
#include <stdexcept>

vkr::CommandBuffer::CommandBuffer(std::shared_ptr<vko::CommandBuffers> const& container, std::size_t index)
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
    VKR_ASSERT(vkResetCommandBuffer(getHandle(), 0));
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

void vkr::CommandBuffer::submit(vko::Queue const& queue, vko::Semaphore const* signalSemaphore, vko::Semaphore const* waitSemaphore, vko::Fence const* signalFence) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &getHandle();

    std::vector<VkSemaphore> signalSemaphores;
    if (signalSemaphore)
        signalSemaphores.push_back(signalSemaphore->getHandle());
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;
    if (waitSemaphore)
    {
        waitSemaphores.push_back(waitSemaphore->getHandle());
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());;;
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    VkFence signalFenceHandle = signalFence ? signalFence->getHandle() : VK_NULL_HANDLE;

    if (auto res = vkQueueSubmit(queue.getHandle(), 1, &submitInfo, signalFenceHandle); res != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer: " + std::to_string(res));
}

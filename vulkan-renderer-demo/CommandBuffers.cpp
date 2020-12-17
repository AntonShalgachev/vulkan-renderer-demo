#include "CommandBuffers.h"

#include "Renderer.h"
#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"

vkr::CommandBuffers::CommandBuffers(Application const& app, std::size_t size) : Object(app)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = temp::getRenderer()->getCommandPool();
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(size);

    m_handles.resize(size);
    vkAllocateCommandBuffers(getDevice().getHandle(), &commandBufferAllocateInfo, m_handles.data());
}

vkr::CommandBuffers::~CommandBuffers()
{
    vkFreeCommandBuffers(getDevice().getHandle(), temp::getRenderer()->getCommandPool(), static_cast<uint32_t>(m_handles.size()), m_handles.data());
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

    vkQueueSubmit(temp::getRenderer()->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(temp::getRenderer()->getGraphicsQueue());
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

    if (vkQueueSubmit(temp::getRenderer()->getGraphicsQueue(), 1, &submitInfo, signalFence.getHandle()) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");
}

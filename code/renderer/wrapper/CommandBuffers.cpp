#include "CommandBuffers.h"

#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"
#include "CommandPool.h"
#include "QueueFamily.h"
#include "Queue.h"

vko::CommandBuffers::CommandBuffers(Device const& device, CommandPool const& commandPool, std::size_t size)
    : m_device(device)
    , m_commandPool(commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = m_commandPool.getHandle();
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(size);

    std::vector<VkCommandBuffer> rawHandles;
    rawHandles.resize(size);
    VKR_ASSERT(vkAllocateCommandBuffers(m_device.getHandle(), &commandBufferAllocateInfo, rawHandles.data()));

    m_handles.reserve(size);
    for (VkCommandBuffer handle : rawHandles)
        m_handles.emplace_back(handle);
}

vko::CommandBuffers::~CommandBuffers()
{
    if (m_handles.empty())
        return;

    // TODO do we need to free command buffers explicitly?
    std::vector<VkCommandBuffer> rawHandles;
    rawHandles.reserve(m_handles.size());
    for (VkCommandBuffer handle : m_handles)
        rawHandles.push_back(handle);
    vkFreeCommandBuffers(m_device.getHandle(), m_commandPool.getHandle(), static_cast<uint32_t>(rawHandles.size()), rawHandles.data());
}

void vko::CommandBuffers::reset(std::size_t index) const
{
    VKR_ASSERT(vkResetCommandBuffer(m_handles[index], 0));
}

void vko::CommandBuffers::begin(std::size_t index, bool oneTime /*= true*/) const
{
    VkCommandBufferUsageFlags flags = 0;
    if (oneTime)
        flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_handles[index], &commandBufferBeginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
}

void vko::CommandBuffers::end(std::size_t index) const
{
    if (vkEndCommandBuffer(m_handles[index]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void vko::CommandBuffers::submit(std::size_t index, Queue const& queue, Semaphore const* signalSemaphore, Semaphore const* waitSemaphore, Fence const* signalFence) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_handles[index].get();

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
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    VkFence signalFenceHandle = signalFence ? signalFence->getHandle() : VK_NULL_HANDLE;

    if (auto res = vkQueueSubmit(queue.getHandle(), 1, &submitInfo, signalFenceHandle); res != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer: " + std::to_string(res));
}

#include "CommandBuffers.h"

#include "vko/Assert.h"
#include "vko/Semaphore.h"
#include "vko/Fence.h"
#include "vko/Device.h"
#include "vko/CommandPool.h"
#include "vko/QueueFamily.h"
#include "vko/Queue.h"

#include "nstl/static_vector.h"

vko::CommandBuffers::CommandBuffers(Device const& device, CommandPool const& commandPool, size_t size)
    : m_device(device.getHandle())
    , m_commandPool(commandPool.getHandle())
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = m_commandPool;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(size);

    nstl::vector<VkCommandBuffer> rawHandles;
    rawHandles.resize(size);
    VKO_VERIFY(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, rawHandles.data()));

    m_handles.reserve(size);
    for (VkCommandBuffer handle : rawHandles)
        m_handles.emplace_back(handle);
}

vko::CommandBuffers::~CommandBuffers()
{
    if (m_handles.empty())
        return;

    // TODO do we need to free command buffers explicitly?
    nstl::vector<VkCommandBuffer> rawHandles;
    rawHandles.reserve(m_handles.size());
    for (VkCommandBuffer handle : m_handles)
        rawHandles.push_back(handle);
    vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(rawHandles.size()), rawHandles.data());
}

void vko::CommandBuffers::reset(size_t index) const
{
    VKO_VERIFY(vkResetCommandBuffer(m_handles[index], 0));
}

void vko::CommandBuffers::begin(size_t index, bool oneTime /*= true*/) const
{
    VkCommandBufferUsageFlags flags = 0;
    if (oneTime)
        flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    VKO_VERIFY(vkBeginCommandBuffer(m_handles[index], &commandBufferBeginInfo));
}

void vko::CommandBuffers::end(size_t index) const
{
    VKO_VERIFY(vkEndCommandBuffer(m_handles[index]));
}

void vko::CommandBuffers::submit(size_t index, Queue const& queue, Semaphore const* signalSemaphore, Semaphore const* waitSemaphore, Fence const* signalFence) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_handles[index].get();

    nstl::static_vector<VkSemaphore, 1> signalSemaphores;
    if (signalSemaphore)
        signalSemaphores.push_back(signalSemaphore->getHandle());
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    nstl::static_vector<VkSemaphore, 1> waitSemaphores;
    nstl::static_vector<VkPipelineStageFlags, 1> waitStages;
    if (waitSemaphore)
    {
        waitSemaphores.push_back(waitSemaphore->getHandle());
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    VkFence signalFenceHandle = signalFence ? signalFence->getHandle() : VK_NULL_HANDLE;

    VKO_VERIFY(vkQueueSubmit(queue.getHandle(), 1, &submitInfo, signalFenceHandle));
}

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

VkCommandBuffer vkr::CommandBuffer::getHandle() const
{
    return m_container->getHandle(m_index);
}

void vkr::CommandBuffer::reset() const
{
    return m_container->reset(m_index);
}

void vkr::CommandBuffer::begin(bool oneTime) const
{
    return m_container->begin(m_index, oneTime);
}

void vkr::CommandBuffer::end() const
{
    return m_container->end(m_index);
}

void vkr::CommandBuffer::submit(vko::Queue const& queue, vko::Semaphore const* signalSemaphore, vko::Semaphore const* waitSemaphore, vko::Fence const* signalFence) const
{
    return m_container->submit(m_index, queue, signalSemaphore, waitSemaphore, signalFence);
}

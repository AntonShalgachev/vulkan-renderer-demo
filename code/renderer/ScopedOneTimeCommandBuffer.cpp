#include "ScopedOneTimeCommandBuffer.h"
#include "wrapper/Device.h"
#include "Application.h"
#include "wrapper/CommandPool.h"
#include "wrapper/Queue.h"

// TODO pass command pool and queue explicitly?
vkr::ScopedOneTimeCommandBuffer::ScopedOneTimeCommandBuffer(Application const& app)
    : Object(app)
    , m_commandBuffers(getApp().getShortLivedCommandPool().createCommandBuffers(1))
{
    m_commandBuffers.begin(0, true);
}

vkr::ScopedOneTimeCommandBuffer::~ScopedOneTimeCommandBuffer()
{
    if (!m_submitted)
        submit();
}

void vkr::ScopedOneTimeCommandBuffer::submit()
{
    vko::Queue const& queue = getApp().getDevice().getGraphicsQueue();

    m_commandBuffers.end(0);
    m_commandBuffers.submit(0, queue, nullptr, nullptr, nullptr);
    queue.waitIdle();

    m_submitted = true;
}

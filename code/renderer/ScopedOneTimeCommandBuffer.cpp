#include "ScopedOneTimeCommandBuffer.h"
#include "wrapper/Device.h"
#include "Application.h"
#include "wrapper/CommandPool.h"
#include "wrapper/Queue.h"

// TODO pass command pool and queue explicitly?
vkr::ScopedOneTimeCommandBuffer::ScopedOneTimeCommandBuffer(Application const& app)
    : Object(app)
    , m_commandBuffer(getApp().getShortLivedCommandPool().createCommandBuffer())
{
    m_commandBuffer.begin(true);
}

vkr::ScopedOneTimeCommandBuffer::~ScopedOneTimeCommandBuffer()
{
    submit();
}

void vkr::ScopedOneTimeCommandBuffer::submit()
{
    if (m_submitted)
        return;

    vko::Queue const& queue = getApp().getDevice().getGraphicsQueue();

    m_commandBuffer.end();
    m_commandBuffer.submit(queue, nullptr, nullptr, nullptr);
    queue.waitIdle();

    m_submitted = true;
}

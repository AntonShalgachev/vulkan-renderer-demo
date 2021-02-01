#include "ScopedOneTimeCommandBuffer.h"
#include "Device.h"
#include "Application.h"
#include "CommandPool.h"

// TODO pass command pool and queue explicitly?
vkr::ScopedOneTimeCommandBuffer::ScopedOneTimeCommandBuffer(Application const& app)
    : Object(app)
    , m_commandBuffer(getApp().getShortLivedCommandPool().createCommandBuffer())
{
    m_commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

vkr::ScopedOneTimeCommandBuffer::~ScopedOneTimeCommandBuffer()
{
    submit();
}

void vkr::ScopedOneTimeCommandBuffer::submit()
{
    if (m_submitted)
        return;

    m_commandBuffer.end();
    m_commandBuffer.submitAndWait(getApp().getDevice().getGraphicsQueue());

    m_submitted = true;
}

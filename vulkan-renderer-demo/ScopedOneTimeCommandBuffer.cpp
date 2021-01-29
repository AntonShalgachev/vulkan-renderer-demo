#include "ScopedOneTimeCommandBuffer.h"
#include "Device.h"
#include "Application.h"

// TODO pass command pool and queue explicitly?
vkr::ScopedOneTimeCommandBuffer::ScopedOneTimeCommandBuffer(Application const& app)
    : Object(app)
    , m_commandBuffers(app, getApp().getCommandPool(), getApp().getDevice().getGraphicsQueue(), 1)
{
    m_commandBuffers.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

vkr::ScopedOneTimeCommandBuffer::~ScopedOneTimeCommandBuffer()
{
    submit();
}

void vkr::ScopedOneTimeCommandBuffer::submit()
{
    if (m_submitted)
        return;

    m_commandBuffers.end(0);
    m_commandBuffers.submitAndWait(0);

    m_submitted = true;
}

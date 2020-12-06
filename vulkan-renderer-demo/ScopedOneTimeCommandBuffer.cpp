#include "ScopedOneTimeCommandBuffer.h"

vkr::ScopedOneTimeCommandBuffer::ScopedOneTimeCommandBuffer()
{
    m_commandBuffers.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

vkr::ScopedOneTimeCommandBuffer::~ScopedOneTimeCommandBuffer()
{
    m_commandBuffers.end(0);
    m_commandBuffers.submitAndWait(0);
}

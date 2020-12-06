#include "Fence.h"

vkr::Fence::Fence()
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(temp::getDevice(), &fenceCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create fence!");
}

vkr::Fence::~Fence()
{
    vkDestroyFence(temp::getDevice(), m_handle, nullptr);
}

void vkr::Fence::wait()
{
    vkWaitForFences(temp::getDevice(), 1, &m_handle, VK_TRUE, UINT64_MAX);
}

void vkr::Fence::reset()
{
    vkResetFences(temp::getDevice(), 1, &m_handle);
}
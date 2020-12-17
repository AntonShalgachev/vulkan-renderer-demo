#include "Fence.h"

vkr::Fence::Fence(Application const& app) : Object(app)
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(temp::getDevice(), &fenceCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create fence!");
}

vkr::Fence::Fence(Fence&& other)
{
    std::swap(m_handle, other.m_handle);
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

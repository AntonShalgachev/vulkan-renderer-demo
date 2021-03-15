#include "Fence.h"
#include "Device.h"

vkr::Fence::Fence(Application const& app) : Object(app)
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(getDevice().getHandle(), &fenceCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create fence!");
}

vkr::Fence::~Fence()
{
    vkDestroyFence(getDevice().getHandle(), m_handle, nullptr);
}

void vkr::Fence::wait() const
{
    vkWaitForFences(getDevice().getHandle(), 1, &m_handle.get(), VK_TRUE, UINT64_MAX);
}

void vkr::Fence::reset() const
{
    vkResetFences(getDevice().getHandle(), 1, &m_handle.get());
}

bool vkr::Fence::isSignaled() const
{
    VkResult result = vkGetFenceStatus(getDevice().getHandle(), m_handle);

    if (result != VK_SUCCESS && result != VK_NOT_READY)
        throw std::runtime_error("unexpected fence status result");

    return result == VK_SUCCESS;
}

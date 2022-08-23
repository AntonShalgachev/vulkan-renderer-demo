#include "Fence.h"
#include "Device.h"
#include <stdexcept>

vko::Fence::Fence(Device const& device) : m_device(device)
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(m_device.getHandle(), &fenceCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create fence!");
}

vko::Fence::~Fence()
{
    vkDestroyFence(m_device.getHandle(), m_handle, nullptr);
}

void vko::Fence::wait() const
{
    VKR_ASSERT(vkWaitForFences(m_device.getHandle(), 1, &m_handle.get(), VK_TRUE, UINT64_MAX));
}

void vko::Fence::reset() const
{
    VKR_ASSERT(vkResetFences(m_device.getHandle(), 1, &m_handle.get()));
}

bool vko::Fence::isSignaled() const
{
    VkResult result = vkGetFenceStatus(m_device.getHandle(), m_handle);

    if (result != VK_SUCCESS && result != VK_NOT_READY)
        throw std::runtime_error("unexpected fence status result");

    return result == VK_SUCCESS;
}

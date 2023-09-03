#include "Fence.h"

#include "vko/Assert.h"
#include "vko/Device.h"

vko::Fence::Fence(Device const& device) : m_device(device)
{
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VKO_VERIFY(vkCreateFence(m_device.getHandle(), &fenceCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::Fence::~Fence()
{
    if (m_handle)
        vkDestroyFence(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}

void vko::Fence::wait() const
{
    VKO_VERIFY(vkWaitForFences(m_device.getHandle(), 1, &m_handle.get(), VK_TRUE, UINT64_MAX));
}

void vko::Fence::reset() const
{
    VKO_VERIFY(vkResetFences(m_device.getHandle(), 1, &m_handle.get()));
}

bool vko::Fence::isSignaled() const
{
    VkResult result = vkGetFenceStatus(m_device.getHandle(), m_handle);
    assert(result == VK_SUCCESS || result == VK_NOT_READY);

    return result == VK_SUCCESS;
}

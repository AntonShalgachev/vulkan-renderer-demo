#include "Semaphore.h"

#include "vko/Assert.h"
#include "vko/Device.h"

vko::Semaphore::Semaphore(Device const& device) : m_device(device)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VKO_VERIFY(vkCreateSemaphore(m_device.getHandle(), &semaphoreCreateInfo, m_allocator, &m_handle.get()));
}

vko::Semaphore::~Semaphore()
{
    if (m_handle)
        vkDestroySemaphore(m_device.getHandle(), m_handle, m_allocator);
}

#include "Semaphore.h"

#include "Assert.h"
#include "Device.h"

vko::Semaphore::Semaphore(Device const& device) : m_device(device)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VKO_ASSERT(vkCreateSemaphore(m_device.getHandle(), &semaphoreCreateInfo, nullptr, &m_handle.get()));
}

vko::Semaphore::~Semaphore()
{
    vkDestroySemaphore(m_device.getHandle(), m_handle, nullptr);
}
